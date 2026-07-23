#include "EnemyBase.h"
#include "game/actor/EnemyBehaviors.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/ai/AIComponent.h"
#include "game/component/combat/HealthComponent.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/TagComponent.h"
#include "game/component/HitEffectComponent.h"
#include "game/component/EffectComponent.h"
#include "game/constant/Tag.h"
#include <utility>

namespace game::actor
{

	EnemyBase::EnemyBase(core::ecs::EntityManager& entityManager,
	    core::ecs::ComponentManager& componentManager,
	    core::iface::IResourceManager& resourceManager,
	    int modelHandle,
	    data::EnemyData enemyData)
	    : m_entity{ entityManager.create() }
	    , m_componentManager{ componentManager }
	    , m_resourceManager{ resourceManager }
	    , m_modelHandle{ modelHandle }
	    , m_enemyData{ std::move(enemyData) }
	{
	}

	void EnemyBase::initialize()
	{
		buildCommonComponents();
		// アニメ・AI振る舞いはEnemyData（JSONのanimations/behaviors）から組み立てる。
		// 敵種ごとの分岐は持たず、データの組み合わせだけで敵の中身が決まる
		installEnemyAnimations(m_componentManager, m_entity.getId(), m_enemyData, m_resourceManager);
		installEnemyBehaviors(m_componentManager, m_entity.getId(), m_enemyData);
	}

	core::ecs::EntityId EnemyBase::getId() const noexcept
	{
		return m_entity.getId();
	}

	void EnemyBase::buildCommonComponents()
	{
		component::TransformComponent transform{};
		transform.m_position = m_enemyData.getPosition();
		transform.m_scale = m_enemyData.getScale();
		m_componentManager.add<component::TransformComponent>(m_entity.getId(), transform);

		m_componentManager.add<component::VelocityComponent>(m_entity.getId(), {});
		m_componentManager.add<component::RenderComponent>(m_entity.getId(), component::RenderComponent{ .m_modelHandle = m_modelHandle });
		m_componentManager.add<component::HitEffectComponent>(m_entity.getId(), {});
		m_componentManager.add<component::EffectComponent>(m_entity.getId(), {});

		component::combat::HealthComponent health{};
		health.m_maxHp = m_enemyData.getMaxHp();
		health.m_currentHp = m_enemyData.getMaxHp();
		health.m_defence = m_enemyData.getDefence();
		m_componentManager.add<component::combat::HealthComponent>(m_entity.getId(), health);

		component::combat::AttackComponent attack{};
		attack.m_attackPower = m_enemyData.getAttackPower();
		attack.m_attackRange = m_enemyData.getAttackRange();
		attack.m_attackCooldown = m_enemyData.getAttackCooldown();
		attack.m_windupDelay = m_enemyData.getAttackWindup();
		m_componentManager.add<component::combat::AttackComponent>(m_entity.getId(), attack);

		component::combat::ColliderComponent collider{};
		collider.m_size = m_enemyData.getColliderSize();
		collider.m_offset = m_enemyData.getColliderOffset();
		m_componentManager.add<component::combat::ColliderComponent>(m_entity.getId(), collider);

		component::TagComponent tag{};
		tag.m_tag = constant::Tag::Enemy;
		m_componentManager.add<component::TagComponent>(m_entity.getId(), tag);

		component::ai::AIComponent ai{};
		ai.m_moveSpeed = m_enemyData.getMoveSpeed();
		ai.m_detectionRange = m_enemyData.getDetectionRange();
		m_componentManager.add<component::ai::AIComponent>(m_entity.getId(), ai);
	}
} // namespace game::actor