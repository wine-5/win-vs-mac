#include "GroundFactory.h"

namespace game::factory
{
	GroundFactory::GroundFactory(
		core::ecs::EntityManager& entityManager,
		core::ecs::ComponentManager& componentManager,
		core::iface::IResourceManager& resourceManager)
		: m_entityManager{entityManager}
		, m_componentManager{componentManager}
		, m_resourceManager{resourceManager}
	{
	}

	core::ecs::EntityId GroundFactory::create(int modelHandle, const data::GroundData& groundData)
	{
		auto ground{std::make_unique<stage::Ground>(
			m_entityManager,
			m_componentManager,
			modelHandle,
			groundData)};

		core::ecs::EntityId id{ground->getId()};
		m_grounds.push_back(std::move(ground));

		return id;
	}

} // namespace game::factory