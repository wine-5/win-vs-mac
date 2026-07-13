#include "FactoryInitializer.h"
#include "core/interface/ILogger.h"
#include <cassert>
#include <stdexcept>
#include "game/constant/EnemyType.h"


namespace game::factory
{
	FactoryInitializer::FactoryInitializer(
		FactoryManager& factoryManager,
		core::iface::IResourceManager& resourceManager)
		: m_factoryManager{factoryManager}
		, m_resourceManager{resourceManager}
	{
	}

	void FactoryInitializer::initializePlayer(const data::PlayerData& playerData)
	{
		int playerHandle{m_resourceManager.loadModelById(constant::model_id::PLAYER)};
		m_factoryManager.getPlayerFactory().create(playerHandle, playerData);
	}

	core::ecs::EntityId FactoryInitializer::initializeGround()
	{
		int groundHandle{m_resourceManager.loadModelById(constant::model_id::GROUND)};
		auto groundMeta{m_resourceManager.getMetadata(constant::model_id::GROUND)};
		if (!groundMeta.has_value()) {
			LOG("ERROR: Groundのメタデータが見つかりません");
			throw std::runtime_error("Groundのメタデータの読み込みに失敗しました");
		}
		assert(groundMeta.has_value() && "Groundのメタデータが見つかりません");

		data::GroundData groundData = data::GroundData::fromMetadata(groundMeta.value());
		return m_factoryManager.getGroundFactory().create(groundHandle, groundData);
	}

	core::ecs::EntityId FactoryInitializer::initializeEnemy()
	{
		int enemyHandle{m_resourceManager.loadModelById(constant::model_id::ENEMY)};
		auto enemyMeta{m_resourceManager.getMetadata(constant::model_id::ENEMY)};

		if (!enemyMeta.has_value())
		{
			LOG("ERROR: Enemyのメタデータが見つかりません");
			throw std::runtime_error("Enemyのメタデータの読み込みに失敗しました");
		}

		data::EnemyData enemyData = data::EnemyData::fromMetadata(enemyMeta.value());
		return m_factoryManager.getEnemyFactory().create(constant::EnemyType::Xcode,enemyHandle, enemyData);
	}
}
