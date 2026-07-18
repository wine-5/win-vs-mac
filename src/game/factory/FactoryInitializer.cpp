#include "FactoryInitializer.h"
#include "core/interface/ILogger.h"
#include <cassert>
#include <stdexcept>
#include "game/constant/EnemyType.h"

namespace
{
	/**
	 * @brief EnemyTypeから対応するモデルIDを取得する
	 * @param type 敵の種類
	 * @return モデルID
	 */
	std::string_view toModelId(game::constant::EnemyType type)
	{
		using game::constant::EnemyType;
		namespace model_id = game::constant::model_id;
		switch (type)
		{
		case EnemyType::Xcode:  return model_id::ENEMY_XCODE;
		case EnemyType::Safari: return model_id::ENEMY_SAFARI;
		case EnemyType::Mac:    return model_id::ENEMY_MAC;
		}
		throw std::runtime_error("未対応のEnemyTypeです");
	}
} // namespace

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

	std::vector<core::ecs::EntityId> FactoryInitializer::initializeEnemies()
	{
		std::vector<core::ecs::EntityId> enemyIds{};
		const auto& stage{ m_resourceManager.getStageMetadata() };
		for (const auto& spawn : stage.m_spawns)
			enemyIds.push_back(spawnEnemy(spawn));
		return enemyIds;
	}

	core::ecs::EntityId FactoryInitializer::spawnEnemy(const core::data::SpawnMetadata& spawn)
	{
		const auto type{ constant::toEnemyType(spawn.m_type) };
		const auto modelId{ toModelId(type) };

		// 同種の敵が複数体いてもアニメーション状態が競合しないよう、複製ハンドルを使う
		int baseHandle{ m_resourceManager.loadModelById(modelId) };
		int modelHandle{ m_resourceManager.duplicateModel(baseHandle) };
		auto meta{ m_resourceManager.getMetadata(modelId) };
		if (!meta.has_value())
		{
			LOG("ERROR: 敵メタデータが見つかりません: {}", std::string(modelId).c_str());
			throw std::runtime_error("敵メタデータの読み込みに失敗しました");
		}

		data::EnemyData enemyData{ data::EnemyData::fromMetadata(meta.value()) };
		enemyData.setPosition(spawn.m_position); // 位置はステージ配置定義が正
		return m_factoryManager.getEnemyFactory().create(type, modelHandle, enemyData);
	}
} // namespace game::factory
