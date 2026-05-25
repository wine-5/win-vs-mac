#include "Player.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/InputComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/AnimationComponent.h"
#include "game/component/ColliderComponent.h"
#include "game/component/TagComponent.h"
#include "game/component/HealthComponent.h"
#include "game/component/AttackComponent.h"
#include "game/component/HitEffectComponent.h"
#include "game/component/EffectComponent.h"
#include "game/constant/Tag.h"
#include "game/constant/PlayerAnimationState.h"

namespace game::actor
{
	Player::Player(core::ecs::EntityManager& entityManager,
		core::ecs::ComponentManager& componentManager,
		int modelHandle,
		const data::PlayerData& playerData)
		: m_entity{entityManager.create()}
	{
		componentManager.add<component::TransformComponent>(m_entity.getId(), {});
		componentManager.add<component::VelocityComponent>(m_entity.getId(), {});
		componentManager.add<component::InputComponent>(m_entity.getId(), {});
		componentManager.add<component::AnimationComponent<constant::PlayerAnimationState>>(m_entity.getId(), {});
		componentManager.add<component::RenderComponent>(m_entity.getId(), { modelHandle });
		componentManager.add<component::HitEffectComponent>(m_entity.getId(), {});
		componentManager.add<component::EffectComponent>(m_entity.getId(), {});

		component::HealthComponent health{};
		health.m_maxHp = playerData.getMaxHp();
		health.m_currentHp = playerData.getMaxHp();
		health.m_defence = playerData.getDefence();
		componentManager.add<component::HealthComponent>(m_entity.getId(), health);

		component::AttackComponent attack{};
		attack.m_attackPower = playerData.getAttackPower();
		attack.m_attackRange = playerData.getAttackRange();
		attack.m_attackCooldown = playerData.getAttackCooldown();
		componentManager.add<component::AttackComponent>(m_entity.getId(), attack);
		component::ColliderComponent collider;
		collider.m_size = playerData.getColliderSize();
		collider.m_offset = playerData.getColliderOffset();
		componentManager.add<component::ColliderComponent>(m_entity.getId(), collider);

		component::TagComponent tag{};
		tag.m_tag = constant::Tag::Player;
		componentManager.add<component::TagComponent>(m_entity.getId(), tag);
	}

	core::ecs::EntityId Player::getId() const noexcept
	{
		return m_entity.getId();
	}
}