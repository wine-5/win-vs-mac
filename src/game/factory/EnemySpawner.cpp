#include "EnemySpawner.h"
#include "FactoryManager.h"
#include "core/interface/ILogger.h"
#include "game/component/AIComponent.h"
#include "game/constant/ModelId.h"
#include <stdexcept>

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
		case EnemyType::Xcode: return model_id::ENEMY_XCODE;
		case EnemyType::Safari: return model_id::ENEMY_SAFARI;
		case EnemyType::Mac: return model_id::ENEMY_MAC;
		}
		throw std::runtime_error("未対応のEnemyTypeです");
	}
} // namespace

namespace game::factory
{
	EnemySpawner::EnemySpawner(
	    FactoryManager& factoryManager,
	    core::ecs::ComponentManager& componentManager,
	    core::iface::IResourceManager& resourceManager)
	    : m_factoryManager{ factoryManager }
	    , m_componentManager{ componentManager }
	    , m_resourceManager{ resourceManager }
	{
	}

	void EnemySpawner::setTargetEntity(core::ecs::Entity target) noexcept
	{
		m_target = target;
	}

	core::ecs::EntityId EnemySpawner::spawn(constant::EnemyType type, const core::Vector3& position)
	{
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
		enemyData.setPosition(position); // 位置は呼び出し側の指定が正
		const auto enemyId{ m_factoryManager.getEnemyFactory().create(type, modelHandle, enemyData) };

		// 追跡対象が設定されていれば反映する（召喚された敵も即プレイヤーを追う）
		if (m_target.getId() != 0 && m_componentManager.has<component::AIComponent>(enemyId))
		{
			auto& ai = m_componentManager.get<component::AIComponent>(enemyId);
			ai.m_targetEntity = m_target;
		}

		return enemyId;
	}

	void EnemySpawner::spawnStageEnemies()
	{
		const auto& stage{ m_resourceManager.getStageMetadata() };
		for (const auto& spawn : stage.m_spawns)
			this->spawn(constant::toEnemyType(spawn.m_type), spawn.m_position);
	}
} // namespace game::factory
