#include "FactoryInitializer.h"

namespace game::factory
{
	FactoryInitializer::FactoryInitializer(
		FactoryManager& factoryManager,
		core::iface::IResourceManager& resourceManager)
		: m_factoryManager(factoryManager)
		, m_resourceManager(resourceManager)
	{
	}


	void FactoryInitializer::initializePlayer(const data::PlayerData& playerData)
	{
		int playerHandle = m_resourceManager.loadModelById(constant::model_id::PLAYER);
		m_factoryManager.getPlayerFactory().create(playerHandle, playerData);
	}

	core::ecs::EntityId FactoryInitializer::initializeGround()
	{
		int groundHandle = m_resourceManager.loadModelById(constant::model_id::GROUND);
		auto groundMeta = m_resourceManager.getMetadata(constant::model_id::GROUND);
		assert(groundMeta.has_value() && "Groundのメタデータが見つかりません");

		data::GroundData groundData = data::GroundData::fromMetadata(groundMeta.value());
		return m_factoryManager.getGroundFactory().create(groundHandle, groundData);
	}
}
