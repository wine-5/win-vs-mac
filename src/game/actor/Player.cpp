#include "Player.h"
#include "game/ecs/component/TransformComponent.h"
#include "game/ecs/component/VelocityComponent.h"
#include "game/ecs/component/InputComponent.h"

namespace game::actor
{
	Player::Player(ecs::EntityManager& entityManager, ecs::ComponentManager& componentManager)
		: m_entity(entityManager.create())
	{
		componentManager.add<ecs::component::TransformComponent>(m_entity.getId(), {});
		componentManager.add<ecs::component::VelocityComponent>(m_entity.getId(), {});
		componentManager.add<ecs::component::InputComponent>(m_entity.getId(), {});
	}

	ecs::EntityId Player::getId() const
	{
		return m_entity.getId();
	}
}