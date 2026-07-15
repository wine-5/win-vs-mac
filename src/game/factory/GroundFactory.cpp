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

	stage::Ground* GroundFactory::getGroundById(core::ecs::EntityId id) const
	{
		for (const auto& ground : m_grounds)
		{
			if (ground->getId() == id)
				return ground.get();
		}
		return nullptr;
	}

	std::vector<stage::Ground*> GroundFactory::getAllGrounds() const
	{
		std::vector<stage::Ground*> grounds;
		for (const auto& ground : m_grounds)
		{
			grounds.push_back(ground.get());
		}
		return grounds;
	}

	size_t GroundFactory::getCount() const
	{
		return m_grounds.size();
	}
} // namespace game::factory