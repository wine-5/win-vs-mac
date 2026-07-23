#include "DamageShakeSystem.h"
#include "game/component/camera/CameraEffectComponent.h"
#include "game/event/InGameEvents.h"
#include <cmath>
#include <algorithm>

namespace
{
	constexpr float SHAKE_DURATION{ 0.35f };         // 1回の揺れの継続時間（秒）
	constexpr float SHAKE_BASE_STRENGTH{ 8.0f };     // 最小ダメージでも出る揺れの振幅（ワールド単位）
	constexpr float SHAKE_DAMAGE_SCALE{ 0.6f };      // ダメージ1あたり増える振幅
	constexpr float SHAKE_MAX_STRENGTH{ 40.0f };     // 振幅の上限（大ダメージでも揺れすぎない）
	constexpr float SHAKE_FREQUENCY{ 40.0f };        // 揺れの振動周波数（大きいほど細かく震える）
	constexpr float SHAKE_SEED_PHASE_STEP{ 10.0f };  // 起動ごとに位相をずらす量（毎回違う揺れにする）
	constexpr float SHAKE_Y_FREQUENCY_RATIO{ 1.3f }; // Y軸をX軸と別周波数にする比（円状でなく不規則な揺れにする）
} // namespace

namespace game::system::camera
{
	DamageShakeSystem::DamageShakeSystem(core::ecs::ComponentManager& componentManager,
	    core::base::EventBus& eventBus,
	    core::ecs::EntityId playerId)
	    : m_componentManager{ componentManager }
	    , m_playerId{ playerId }
	{
		// プレイヤーが被弾したら揺れを起動する
		m_subscriptions.push_back(eventBus.subscribe<event::AttackHitEvent>(
		    [this](const event::AttackHitEvent& e)
		    {
			    if (e.m_targetId != m_playerId)
				    return;

			    // 揺れ中に再度被弾したら、より強い方を採用して上書きする（弱い揺れで打ち消さない）
			    const float strength{ std::min(SHAKE_BASE_STRENGTH + e.m_damage * SHAKE_DAMAGE_SCALE, SHAKE_MAX_STRENGTH) };
			    if (strength < m_strength && m_remainingTime > 0.0f)
				    return;

			    m_strength = strength;
			    m_duration = SHAKE_DURATION;
			    m_remainingTime = SHAKE_DURATION;
			    m_seed += 1.0f; // 起動ごとに位相をずらし、毎回違う揺れにする
		    }));
	}

	void DamageShakeSystem::update(float deltaTime)
	{
		if (!m_componentManager.has<component::camera::CameraEffectComponent>(m_playerId))
			return;

		auto& effect{ m_componentManager.get<component::camera::CameraEffectComponent>(m_playerId) };

		if (m_remainingTime <= 0.0f)
		{
			effect.m_shakeOffset = core::Vector3{ 0.0f, 0.0f, 0.0f };
			return;
		}

		m_remainingTime -= deltaTime;
		if (m_remainingTime < 0.0f)
			m_remainingTime = 0.0f;

		// 残り時間の比率で線形に減衰させる（開始時が最大、終了時にゼロ）
		const float decay{ m_duration > 0.0f ? m_remainingTime / m_duration : 0.0f };
		const float amplitude{ m_strength * decay };

		// 経過時間から位相を作り、X/Yで別の周波数にして円ではなく不規則な揺れにする
		const float phase{ (m_duration - m_remainingTime) * SHAKE_FREQUENCY + m_seed * SHAKE_SEED_PHASE_STEP };
		effect.m_shakeOffset = core::Vector3{
			std::sin(phase) * amplitude,
			std::cos(phase * SHAKE_Y_FREQUENCY_RATIO) * amplitude,
			0.0f
		};
	}
} // namespace game::system::camera
