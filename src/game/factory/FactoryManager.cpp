#include "FactoryManager.h"

namespace game::factory
{
	FactoryManager::FactoryManager(
	    core::ecs::EntityManager& entityManager,
	    core::ecs::ComponentManager& componentManager,
	    core::iface::IResourceManager& resourceManager)
	    : m_playerFactory(std::make_unique<PlayerFactory>(entityManager, componentManager, resourceManager))
	    , m_groundFactory(std::make_unique<GroundFactory>(entityManager, componentManager, resourceManager))
	    , m_stagePropFactory(std::make_unique<StagePropFactory>(entityManager, componentManager))
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

	StagePropFactory& FactoryManager::getStagePropFactory() const noexcept
	{
		return *m_stagePropFactory;
	}

	EnemyFactory& FactoryManager::getEnemyFactory() const noexcept
	{
		return *m_enemyFactory;
	}
} // namespace game::factory