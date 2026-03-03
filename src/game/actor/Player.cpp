#include "Player.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/InputComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/AnimationComponent.h"
#include "game/constant/PlayerAnimationState.h"

namespace game::actor
{
	Player::Player(core::ecs::EntityManager& entityManager,
		core::ecs::ComponentManager& componentManager,
		int modelHandle)
		: m_entity(entityManager.create())
	{
		componentManager.add<component::TransformComponent>(m_entity.getId(), {});
		componentManager.add<component::VelocityComponent>(m_entity.getId(), {});
		componentManager.add<component::InputComponent>(m_entity.getId(), {});
		componentManager.add<component::AnimationComponent<constant::PlayerAnimationState>>(m_entity.getId(), {});

		componentManager.add<component::RenderComponent>(m_entity.getId(), { modelHandle });
	}

	core::ecs::EntityId Player::getId() const
	{
		return m_entity.getId();
	}
}