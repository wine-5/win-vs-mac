#include "ObjectFactory.h"

namespace game
{
	ObjectFactory::ObjectFactory(core::ecs::EntityManager& entityManager, core::ecs::ComponentManager& componentManager, core::iface::IResourceManager& resourceManager)
		: m_entityManager(entityManager)
		, m_componentManager(componentManager)
		, m_resourceManager(resourceManager)
	{
	}

	void ObjectFactory::init()
	{
		m_player = std::make_unique<actor::Player>(m_entityManager, m_componentManager, m_resourceManager);
	}

	actor::Player& ObjectFactory::getPlayer() const
	{
		return *m_player; // *でデリファレンスして参照として返す
	}
}