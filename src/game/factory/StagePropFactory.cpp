#include "StagePropFactory.h"

namespace game::factory
{
	StagePropFactory::StagePropFactory(
	    core::ecs::EntityManager& entityManager,
	    core::ecs::ComponentManager& componentManager)
	    : m_entityManager{ entityManager }
	    , m_componentManager{ componentManager }
	{
	}

	core::ecs::EntityId StagePropFactory::create(int modelHandle,
	    const core::Vector3& position,
	    const core::Vector3& rotation,
	    const core::Vector3& scale,
	    constant::PropCollision collision,
	    const core::Vector3& collisionSize)
	{
		auto prop{ std::make_unique<stage::StageProp>(
			m_entityManager,
			m_componentManager,
			modelHandle,
			position,
			rotation,
			scale,
			collision,
			collisionSize) };

		core::ecs::EntityId id{ prop->getId() };
		m_props.push_back(std::move(prop));

		return id;
	}
} // namespace game::factory
