#include "PlayerRangedAttackSystem.h"
#include "game/component/movement/InputComponent.h"
#include "game/component/camera/CameraComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/combat/PlayerChargeComponent.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/visual/AnimationComponent.h"
#include "game/constant/AnimationState.h"
#include "game/constant/Tag.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IAudioManager.h"
#include <algorithm>
#include <utility>

namespace
{
	// 溜め切ったとみなす溜め率。溜め時間は最大値で頭打ちにしてから割るので理屈上は1.0ちょうどだが、
	// 浮動小数の誤差で1.0をわずかに下回ることがあるため手前で判定する
	constexpr float FULL_CHARGE_THRESHOLD{ 0.99f };
} // namespace

namespace game::system::combat
{
	PlayerRangedAttackSystem::PlayerRangedAttackSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId playerId,
	    factory::ProjectileFactory& projectileFactory,
	    core::data::ProjectileMetadata metadata,
	    int billboardImage)
	    : m_componentManager{ componentManager }
	    , m_playerId{ playerId }
	    , m_projectileFactory{ projectileFactory }
	    , m_metadata{ std::move(metadata) }
	    , m_billboardImage{ billboardImage }
	{
	}

	float PlayerRangedAttackSystem::getCooldownRatio() const
	{
		if (m_metadata.m_cooldown <= 0.0f)
			return 0.0f;
		return std::clamp(m_cooldownTimer / m_metadata.m_cooldown, 0.0f, 1.0f);
	}

	void PlayerRangedAttackSystem::update(float deltaTime)
	{
		if (m_cooldownTimer > 0.0f)
			m_cooldownTimer -= deltaTime;

		if (!m_componentManager.has<component::movement::InputComponent>(m_playerId) ||
		    !m_componentManager.has<component::camera::CameraComponent>(m_playerId))
			return;

		const auto& input{ m_componentManager.get<component::movement::InputComponent>(m_playerId) };

		// プレイヤーのPlayerChargeComponentがあれば更新する
		bool hasChargeComponent{ m_componentManager.has<component::combat::PlayerChargeComponent>(m_playerId) };
		if (hasChargeComponent)
		{
			auto& playerCharge{ m_componentManager.get<component::combat::PlayerChargeComponent>(m_playerId) };
			playerCharge.m_isCharging = m_isCharging;
			if (m_metadata.m_chargeMaxTime > 0.0f)
				playerCharge.m_chargeRate = m_chargeTime / m_metadata.m_chargeMaxTime;
			else
				playerCharge.m_chargeRate = 0.0f;

			playerCharge.m_isFullyCharged = m_isCharging && playerCharge.m_chargeRate >= FULL_CHARGE_THRESHOLD;
		}

		// 押している間は溜める（クールダウン中は溜め開始しない）
		if (input.m_rangedAttackPressed)
		{
			if (!m_isCharging && m_cooldownTimer <= 0.0f)
			{
				m_isCharging = true;
				m_chargeTime = 0.0f;
				m_hasNotifiedFullCharge = false;

				// 溜め始めた合図。集中線が出るより先に音で分かるようにする
				auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
				if (audio)
					audio->playSe(core::constant::SeType::PlayerCharge);
			}

			if (m_isCharging)
			{
				m_chargeTime += deltaTime;
				// 最大溜め時間で頭打ちにする
				if (m_chargeTime > m_metadata.m_chargeMaxTime)
					m_chargeTime = m_metadata.m_chargeMaxTime;

				// 溜め切った瞬間に1回だけ合図を鳴らす。ここから先は溜めても強くならないため、
				// 画面を見ていなくても「今離せば最大」が耳で分かるようにする
				const bool isFull{ m_metadata.m_chargeMaxTime > 0.0f &&
					               m_chargeTime / m_metadata.m_chargeMaxTime >= FULL_CHARGE_THRESHOLD };
				if (isFull && !m_hasNotifiedFullCharge)
				{
					m_hasNotifiedFullCharge = true;
					auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
					if (audio)
						audio->playSe(core::constant::SeType::PlayerChargeReady);
				}
			}
			return;
		}

		// ボタンを離した瞬間に発射する
		if (m_isCharging)
		{
			// 溜め率（0.0〜1.0）。溜め無効（chargeMaxTime==0）なら常に0
			const float chargeRate{ m_metadata.m_chargeMaxTime > 0.0f
				                        ? m_chargeTime / m_metadata.m_chargeMaxTime
				                        : 0.0f };
			fire(chargeRate);
			m_isCharging = false;
			m_chargeTime = 0.0f;
			m_hasNotifiedFullCharge = false;
			m_cooldownTimer = m_metadata.m_cooldown;
		}
	}

	void PlayerRangedAttackSystem::fire(float chargeRate)
	{
		// 投擲モーションを再生する。溜めの有無で動きは変えないため、
		// 溜め撃ちも通常撃ちも同じクリップを使う
		if (m_componentManager.has<component::visual::AnimationComponent>(m_playerId))
			m_componentManager.get<component::visual::AnimationComponent>(m_playerId)
			    .request(constant::AnimationState::Throw);

		const auto& camera{ m_componentManager.get<component::camera::CameraComponent>(m_playerId) };
		const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(m_playerId) };

		// 溜め率に応じて倍率を線形補間する（0で等倍、1で最大倍率）
		const float damageMultiplier{ 1.0f + (m_metadata.m_chargeDamageMultiplier - 1.0f) * chargeRate };
		const float sizeMultiplier{ 1.0f + (m_metadata.m_chargeSizeMultiplier - 1.0f) * chargeRate };
		// 弾速と寿命は個別に設定できる（飛距離は両者の積で決まる）
		const float speedMultiplier{ 1.0f + (m_metadata.m_chargeSpeedMultiplier - 1.0f) * chargeRate };
		const float lifetimeMultiplier{ 1.0f + (m_metadata.m_chargeLifetimeMultiplier - 1.0f) * chargeRate };

		// カメラ前方へ、プレイヤーの少し前・目線の高さから発射する
		const core::Vector3 direction{ camera.m_forward };
		const core::Vector3 origin{
			transform.m_position.x + direction.x * m_metadata.m_spawnForward,
			transform.m_position.y + m_metadata.m_spawnHeight + direction.y * m_metadata.m_spawnForward,
			transform.m_position.z + direction.z * m_metadata.m_spawnForward
		};

		factory::ProjectileConfig config{};
		config.m_speed = m_metadata.m_speed * speedMultiplier;
		config.m_damage = m_metadata.m_damage * damageMultiplier;
		config.m_lifetime = m_metadata.m_lifetime * lifetimeMultiplier;
		config.m_radius = m_metadata.m_radius * sizeMultiplier;
		config.m_scale = m_metadata.m_scale;

		// 見た目は板に貼ったWindow画像（ビルボード）。当たり判定半径に合わせて大きさを決め、
		// 溜めサイズ倍率も反映する。視認しやすいよう当たり判定より少し大きめにする
		constexpr float BILLBOARD_SIZE_FACTOR{ 2.5f };
		config.m_billboardImage = m_billboardImage;
		config.m_billboardSize = m_metadata.m_radius * BILLBOARD_SIZE_FACTOR * sizeMultiplier;

		// 溜め切って撃った弾だけ重い着弾音にする。溜めた甲斐を音でも返すため、
		// 見た目（サイズ）と同じく「溜め切ったか」で切り替える
		const bool isFullyCharged{ chargeRate >= FULL_CHARGE_THRESHOLD };
		config.m_hitSeType = isFullyCharged
		                         ? core::constant::SeType::HitChargedWindow
		                         : core::constant::SeType::HitWindow;

		// 溜め切って撃ったときだけ専用の発射音を鳴らす。通常撃ちは今までどおり無音のままにして、
		// 「溜め切った一撃」であることを離した瞬間に耳で分かるようにする
		if (isFullyCharged)
		{
			if (auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() })
				audio->playSe(core::constant::SeType::PlayerChargeRelease);
		}

		// 壁・ブロックを抜けられるのは溜め切った弾だけ。通常撃ちは遮蔽物で止まる
		config.m_penetratesWalls = isFullyCharged;

		// 近接と同じようにクリティカルが出るよう、プレイヤーの会心設定を弾へ引き継ぐ
		if (auto* attack{ m_componentManager.tryGet<component::combat::AttackComponent>(m_playerId) })
		{
			config.m_criticalRate = attack->m_criticalRate;
			config.m_criticalMultiplier = attack->m_criticalMultiplier;
		}

		m_projectileFactory.spawn(origin, direction, config, constant::Tag::Player);
	}
} // namespace game::system::combat
