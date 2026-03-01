#include "ObjectFactory.h"

namespace game
{
	ObjectFactory::ObjectFactory(core::ecs::EntityManager& entityManager, core::ecs::ComponentManager& componentManager)
		: m_entityManager(entityManager)
		, m_componentManager(componentManager)
	{
	}

	void ObjectFactory::init()
	{
		m_player = std::make_unique<actor::Player>(m_entityManager, m_componentManager);
	}

	actor::Player& ObjectFactory::getPlayer() const
	{
		return *m_player; // *でデリファレンスして参照として返す
	}
}