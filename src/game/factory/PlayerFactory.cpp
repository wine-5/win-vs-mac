#include "PlayerFactory.h"

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
		return *m_player;
	}
} // namespace game::factory