#include "FootstepSystem.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/combat/HealthComponent.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IAudioManager.h"
#include "core/constant/SeType.h"
#include <cmath>

namespace
{
	// 一歩ぶんの歩幅（ワールド単位）。歩き速度300でおよそ毎秒2.7歩になる値。
	// 大きくすると音が間延びし、小さくすると細かく鳴りすぎる
	constexpr float STRIDE_DISTANCE{ 110.0f };

	// 押し戻しやワープでの位置の飛びを歩数に数えないための上限（ワールド単位）。
	// 1フレームでこれ以上動いたぶんは移動ではないとみなす
	constexpr float MAX_STEP_DISTANCE_PER_FRAME{ 60.0f };
} // namespace

namespace game::system::movement
{
	FootstepSystem::FootstepSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId playerId)
	    : m_componentManager{ componentManager }
	    , m_playerId{ playerId }
	{
	}

	void FootstepSystem::update(float /*deltaTime*/)
	{
		if (!m_componentManager.has<component::movement::TransformComponent>(m_playerId) ||
		    !m_componentManager.has<component::movement::VelocityComponent>(m_playerId))
			return;

		const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(m_playerId) };
		const auto& velocity{ m_componentManager.get<component::movement::VelocityComponent>(m_playerId) };

		const core::Vector3 position{ transform.m_position };
		const bool hadLastPosition{ m_hasLastPosition };
		const core::Vector3 lastPosition{ m_lastPosition };
		m_lastPosition = position;
		m_hasLastPosition = true;

		if (!hadLastPosition)
			return;

		// 倒れたあとは足音を鳴らさない（死亡アニメでモデルが動くため）
		if (const auto* health{ m_componentManager.tryGet<component::combat::HealthComponent>(m_playerId) })
		{
			if (health->m_isDead)
				return;
		}

		// 空中にいる間は歩数を数えない。着地したらすぐ一歩目が出るよう、溜めた距離は捨てる
		if (!velocity.m_isGrounded)
		{
			m_distanceSinceStep = STRIDE_DISTANCE;
			return;
		}

		// 高さの変化は歩数に関係ない（坂を下っても歩幅は変わらない）ので水平距離だけ見る
		const float dx{ position.x - lastPosition.x };
		const float dz{ position.z - lastPosition.z };
		const float distance{ std::sqrt(dx * dx + dz * dz) };
		if (distance > MAX_STEP_DISTANCE_PER_FRAME)
			return;

		m_distanceSinceStep += distance;
		if (m_distanceSinceStep < STRIDE_DISTANCE)
			return;

		// 溜まったぶんを丸ごと捨てずに歩幅だけ引き、速く動いても歩数がずれないようにする
		m_distanceSinceStep -= STRIDE_DISTANCE;

		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio)
			audio->playSe(core::constant::SeType::PlayerFootstep);
	}
} // namespace game::system::movement
