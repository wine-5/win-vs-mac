#include "Enemy.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/AnimationComponent.h"
#include "game/component/ColliderComponent.h"
#include "game/component/AIComponent.h"
#include "game/component/HealthComponent.h"
#include "game/component/AttackComponent.h"
#include "game/constant/EnemyAnimationState.h"
#include "game/constant/CollisionTag.h"

namespace game::actor
{
	Enemy::Enemy(core::ecs::EntityManager& entityManager,
		core::ecs::ComponentManager& componentManager,
		int modelHandle,
		const data::EnemyData& enemyData)
		: m_entity{ entityManager.create() }
	{
		componentManager.add<component::TransformComponent>(m_entity.getId(), {});
		componentManager.add<component::VelocityComponent>(m_entity.getId(), {});
		componentManager.add<component::AnimationComponent<constant::EnemyAnimationState>>(m_entity.getId(), {});
		componentManager.add<component::RenderComponent>(m_entity.getId(), { modelHandle });
		componentManager.add<component::HealthComponent>(m_entity.getId(), {});
		componentManager.add<component::AttackComponent>(m_entity.getId(), {});

		component::ColliderComponent collider;
		collider.m_size = enemyData.getColliderSize();
		collider.m_offset = enemyData.getColliderOffset();
		collider.m_tag = constant::CollisionTag::Enemy;
		componentManager.add<component::ColliderComponent>(m_entity.getId(), collider);

		component::AIComponent ai;
		ai.m_moveSpeed = enemyData.getMoveSpeed();
		ai.m_detectionRange = enemyData.getDetectionRange();
		ai.m_attackRange = enemyData.getAttackRange();
		componentManager.add<component::AIComponent>(m_entity.getId(), ai);
	}

	core::ecs::EntityId Enemy::getId() const noexcept
	{
		return m_entity.getId();
	}
}