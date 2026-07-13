#include "EnemyFactory.h"

namespace game::factory
{
	EnemyFactory::EnemyFactory(
		core::ecs::EntityManager& entityManager,
		core::ecs::ComponentManager& componentManager,
		core::iface::IResourceManager& resourceManager)
		: m_entityManager{ entityManager }
		, m_componentManager{ componentManager }
		, m_resourceManager{ resourceManager }
	{
	}

	core::ecs::EntityId EnemyFactory::create(int modelHandle, const data::EnemyData& enemyData)
	{
		auto enemy{std::make_unique<actor::Enemy>(
			m_entityManager,
			m_componentManager,
			m_resourceManager,
			modelHandle,
			enemyData)};

		core::ecs::EntityId id{enemy->getId()};
		m_enemies.push_back(std::move(enemy));
		m_enemyIds.push_back(id);
		return id;
	}

	const std::vector<core::ecs::EntityId>& EnemyFactory::getEnemyIds() const noexcept
	{
		return m_enemyIds;
	}
}