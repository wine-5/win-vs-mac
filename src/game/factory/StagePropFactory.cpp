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

	core::ecs::EntityId StagePropFactory::create(const stage::StagePropParams& params)
	{
		auto prop{ std::make_unique<stage::StageProp>(
			m_entityManager,
			m_componentManager,
			params) };

		core::ecs::EntityId id{ prop->getId() };
		m_props.push_back(std::move(prop));

		return id;
	}
} // namespace game::factory
