#include "PlayerFactory.h"
#include <cassert>

namespace game::factory
{
	PlayerFactory::PlayerFactory(
		core::ecs::EntityManager& entityManager,
		core::ecs::ComponentManager& componentManager,
		core::iface::IResourceManager& resourceManager)
		: m_entityManager{entityManager}
		, m_componentManager{componentManager}
		, m_resourceManager{resourceManager}
	{
	}

	void PlayerFactory::create(int modelHandle, const data::PlayerData& playerData)
	{
		m_player = std::make_unique<actor::Player>(
			m_entityManager,
			m_componentManager,
			m_resourceManager,
			modelHandle,
			playerData);
	}

	actor::Player& PlayerFactory::getPlayer() const
	{
		assert(m_player && "PlayerFactory::getPlayer(): create()より前に呼ばれました");
		return *m_player;
	}
} // namespace game::factory