#include "ProjectileFactory.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/AttackComponent.h"
#include "game/component/TagComponent.h"
#include "game/component/ProjectileComponent.h"
#include "game/component/RenderComponent.h"

namespace game::factory
{
	ProjectileFactory::ProjectileFactory(core::ecs::EntityManager& entityManager,
	    core::ecs::ComponentManager& componentManager)
	    : m_entityManager{ entityManager }, m_componentManager{ componentManager }
	{
	}

	core::ecs::EntityId ProjectileFactory::spawn(const core::Vector3& origin,
	    const core::Vector3& direction,
	    const ProjectileConfig& config,
	    constant::Tag ownerTag)
	{
		const core::ecs::Entity entity{ m_entityManager.create() };
		const core::ecs::EntityId id{ entity.getId() };

		component::TransformComponent transform{};
		transform.m_position = origin;
		transform.m_scale = { config.m_scale, config.m_scale, config.m_scale };
		m_componentManager.add<component::TransformComponent>(id, transform);

		// 速度＝方向×速さ。PhysicsSystemが位置を更新する（弾は重力を受けない）
		component::VelocityComponent velocity{};
		velocity.m_velocity = {
			direction.x * config.m_speed,
			direction.y * config.m_speed,
			direction.z * config.m_speed
		};
		m_componentManager.add<component::VelocityComponent>(id, velocity);

		// 当たり判定攻撃は AttackComponent + AttackSystem を流用する
		// attackRange＝接触半径、attackPower＝ダメージ、cooldown0＋requested維持で常時ヒット判定
		component::AttackComponent attack{};
		attack.m_attackPower = config.m_damage;
		attack.m_attackRange = config.m_radius;
		attack.m_attackCooldown = 0.0f;
		attack.m_attackRequested = true;
		m_componentManager.add<component::AttackComponent>(id, attack);

		// 発射者の陣営（AttackSystemが同陣営を弾く＝誤爆防止）
		component::TagComponent tag{};
		tag.m_tag = ownerTag;
		m_componentManager.add<component::TagComponent>(id, tag);

		// 弾固有のデータ（残り寿命・発射位置・見た目）。これがあることでProjectileSystem/PhysicsSystemが弾として扱う
		component::ProjectileComponent projectile{};
		projectile.m_remainingLifetime = config.m_lifetime;
		projectile.m_spawnPosition = origin;
		m_componentManager.add<component::ProjectileComponent>(id, projectile);

		// 3Dモデルの弾（Safariのタブ等）はRenderComponentを付与し、InGameViewが回転描画する
		if (config.m_modelHandle != -1)
			m_componentManager.add<component::RenderComponent>(id, { config.m_modelHandle, true });

		return id;
	}
} // namespace game::factory