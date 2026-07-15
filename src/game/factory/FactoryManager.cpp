#include "FactoryManager.h"

namespace game::factory
{
	FactoryManager::FactoryManager(
		core::ecs::EntityManager& entityManager,
		core::ecs::ComponentManager& componentManager,
		core::iface::IResourceManager& resourceManager)
		: m_playerFactory(std::make_unique<PlayerFactory>(entityManager, componentManager, resourceManager))
		, m_groundFactory(std::make_unique<GroundFactory>(entityManager, componentManager, resourceManager))
		, m_enemyFactory(std::make_unique<EnemyFactory>(entityManager, componentManager, resourceManager))
	{
	}

	PlayerFactory& FactoryManager::getPlayerFactory() const noexcept
	{
		return *m_playerFactory;
	}

	GroundFactory& FactoryManager::getGroundFactory() const noexcept
	{
		return *m_groundFactory;
	}

	EnemyFactory& FactoryManager::getEnemyFactory() const noexcept
	{
		return *m_enemyFactory;
	}
} // namespace game::factory