#include "PlayerRangedAttackSystem.h"
#include "game/component/InputComponent.h"
#include "game/component/CameraComponent.h"
#include "game/component/TransformComponent.h"
#include "game/component/combat/PlayerChargeComponent.h"
#include "game/constant/Tag.h"
#include <utility>

namespace game::system::combat
{
	PlayerRangedAttackSystem::PlayerRangedAttackSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId playerId,
	    factory::ProjectileFactory& projectileFactory,
	    core::data::ProjectileMetadata metadata)
	    : m_componentManager{ componentManager }
	    , m_playerId{ playerId }
	    , m_projectileFactory{ projectileFactory }
	    , m_metadata{ std::move(metadata) }
	{
	}

	void PlayerRangedAttackSystem::update(float deltaTime)
	{
		if (m_cooldownTimer > 0.0f)
			m_cooldownTimer -= deltaTime;

		if (!m_componentManager.has<component::InputComponent>(m_playerId) ||
		    !m_componentManager.has<component::CameraComponent>(m_playerId))
			return;

		const auto& input{ m_componentManager.get<component::InputComponent>(m_playerId) };

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
		}

		// 押している間は溜める（クールダウン中は溜め開始しない）
		if (input.m_rangedAttackPressed)
		{
			if (!m_isCharging && m_cooldownTimer <= 0.0f)
			{
				m_isCharging = true;
				m_chargeTime = 0.0f;
			}

			if (m_isCharging)
			{
				m_chargeTime += deltaTime;
				// 最大溜め時間で頭打ちにする
				if (m_chargeTime > m_metadata.m_chargeMaxTime)
					m_chargeTime = m_metadata.m_chargeMaxTime;
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
			m_cooldownTimer = m_metadata.m_cooldown;
		}
	}

	void PlayerRangedAttackSystem::fire(float chargeRate)
	{
		const auto& camera{ m_componentManager.get<component::CameraComponent>(m_playerId) };
		const auto& transform{ m_componentManager.get<component::TransformComponent>(m_playerId) };

		// 溜め率に応じて倍率を線形補間する（0で等倍、1で最大倍率）
		const float damageMultiplier{ 1.0f + (m_metadata.m_chargeDamageMultiplier - 1.0f) * chargeRate };
		const float sizeMultiplier{ 1.0f + (m_metadata.m_chargeSizeMultiplier - 1.0f) * chargeRate };

		// カメラ前方へ、プレイヤーの少し前・目線の高さから発射する
		const core::Vector3 direction{ camera.m_forward };
		const core::Vector3 origin{
			transform.m_position.x + direction.x * m_metadata.m_spawnForward,
			transform.m_position.y + m_metadata.m_spawnHeight + direction.y * m_metadata.m_spawnForward,
			transform.m_position.z + direction.z * m_metadata.m_spawnForward
		};

		factory::ProjectileConfig config{};
		config.m_speed = m_metadata.m_speed;
		config.m_damage = m_metadata.m_damage * damageMultiplier;
		config.m_lifetime = m_metadata.m_lifetime;
		config.m_radius = m_metadata.m_radius * sizeMultiplier;
		config.m_scale = m_metadata.m_scale;

		m_projectileFactory.spawn(origin, direction, config, constant::Tag::Player);
	}
} // namespace game::system::combat
