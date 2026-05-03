#include "Player.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/InputComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/AnimationComponent.h"
#include "game/component/ColliderComponent.h"
#include "game/component/HealthComponent.h"
#include "game/component/AttackComponent.h"
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
		componentManager.add<component::HealthComponent>(m_entity.getId(), {});
		componentManager.add<component::AttackComponent>(m_entity.getId(), {});
		component::ColliderComponent collider;
		collider.m_size = playerData.getColliderSize();
		collider.m_offset = playerData.getColliderOffset();
		collider.m_tag = constant::CollisionTag::Player;
		componentManager.add<component::ColliderComponent>(m_entity.getId(), collider);
	}

	core::ecs::EntityId Player::getId() const noexcept
	{
		return m_entity.getId();
	}
}