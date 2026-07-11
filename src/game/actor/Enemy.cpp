#include "Enemy.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/AnimationComponent.h"
#include "game/component/ColliderComponent.h"
#include "game/component/AIComponent.h"
#include "game/component/HealthComponent.h"
#include "game/component/AttackComponent.h"
#include "game/component/TagComponent.h"
#include "game/component/HitEffectComponent.h"
#include "game/component/EffectComponent.h"
#include "game/constant/EnemyAnimationState.h"
#include "game/constant/Tag.h"

namespace game::actor
{
	Enemy::Enemy(core::ecs::EntityManager& entityManager,
		core::ecs::ComponentManager& componentManager,
		int modelHandle,
		const data::EnemyData& enemyData)
		: m_entity{ entityManager.create() }
	{
		component::TransformComponent transform{};
		transform.m_scale = enemyData.getScale();
		componentManager.add<component::TransformComponent>(m_entity.getId(), transform);
		componentManager.add<component::VelocityComponent>(m_entity.getId(), {});
		componentManager.add<component::AnimationComponent<constant::EnemyAnimationState>>(m_entity.getId(), {});
		componentManager.add<component::RenderComponent>(m_entity.getId(), { modelHandle });
		componentManager.add<component::HitEffectComponent>(m_entity.getId(), {});
		componentManager.add<component::EffectComponent>(m_entity.getId(), {});

		component::HealthComponent health{};
		health.m_maxHp = enemyData.getMaxHp();
		health.m_currentHp = enemyData.getMaxHp();
		health.m_defence = enemyData.getDefence();
		componentManager.add<component::HealthComponent>(m_entity.getId(), health);

		component::AttackComponent attack{};
		attack.m_attackPower = enemyData.getAttackPower();
		attack.m_attackRange = enemyData.getAttackRange();
		attack.m_attackCooldown = enemyData.getAttackCooldown();
		componentManager.add<component::AttackComponent>(m_entity.getId(), attack);

		component::ColliderComponent collider;
		collider.m_size = enemyData.getColliderSize();
		collider.m_offset = enemyData.getColliderOffset();
		componentManager.add<component::ColliderComponent>(m_entity.getId(), collider);

		component::TagComponent tag{};
		tag.m_tag = constant::Tag::Enemy;
		componentManager.add<component::TagComponent>(m_entity.getId(), tag);

		component::AIComponent ai;
		ai.m_moveSpeed = enemyData.getMoveSpeed();
		ai.m_detectionRange = enemyData.getDetectionRange();
		componentManager.add<component::AIComponent>(m_entity.getId(), ai);
	}

	core::ecs::EntityId Enemy::getId() const noexcept
	{
		return m_entity.getId();
	}
}