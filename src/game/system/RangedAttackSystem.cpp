#include "RangedAttackSystem.h"
#include "game/component/InputComponent.h"
#include "game/component/CameraComponent.h"
#include "game/component/TransformComponent.h"
#include "game/constant/Tag.h"

namespace game::system
{
	RangedAttackSystem::RangedAttackSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId playerId,
	    factory::ProjectileFactory& projectileFactory,
	    const core::data::ProjectileMetadata& metadata,
	    int projectileImageHandle)
	    : m_componentManager{ componentManager }
	    , m_playerId{ playerId }
	    , m_projectileFactory{ projectileFactory }
	    , m_metadata{ metadata }
	    , m_projectileImageHandle{ projectileImageHandle }
	{
	}

	void RangedAttackSystem::update(float deltaTime)
	{
		if (m_cooldownTimer > 0.0f)
			m_cooldownTimer -= deltaTime;

		if (!m_componentManager.has<component::InputComponent>(m_playerId) ||
		    !m_componentManager.has<component::CameraComponent>(m_playerId))
			return;

		const auto& input{ m_componentManager.get<component::InputComponent>(m_playerId) };
		if (!input.m_rangedAttackPressed || m_cooldownTimer > 0.0f)
			return;

		const auto& camera{ m_componentManager.get<component::CameraComponent>(m_playerId) };
		const auto& transform{ m_componentManager.get<component::TransformComponent>(m_playerId) };

		// カメラ前方へ、プレイヤーの少し前・目線の高さから発射する
		const core::Vector3 direction{ camera.m_forward };
		const core::Vector3 origin{
			transform.m_position.x + direction.x * m_metadata.m_spawnForward,
			transform.m_position.y + m_metadata.m_spawnHeight + direction.y * m_metadata.m_spawnForward,
			transform.m_position.z + direction.z * m_metadata.m_spawnForward
		};

		factory::ProjectileConfig config{};
		config.m_speed = m_metadata.m_speed;
		config.m_damage = m_metadata.m_damage;
		config.m_lifetime = m_metadata.m_lifetime;
		config.m_radius = m_metadata.m_radius;
		config.m_scale = m_metadata.m_scale;
		config.m_imageHandle = m_projectileImageHandle;

		m_projectileFactory.spawn(origin, direction, config, constant::Tag::Player);
		m_cooldownTimer = m_metadata.m_cooldown;
	}
} // namespace game::system
