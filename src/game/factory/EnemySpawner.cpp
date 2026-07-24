#include "EnemySpawner.h"
#include "game/component/EnemyTypeComponent.h"
#include "FactoryManager.h"
#include "core/interface/ILogger.h"
#include "core/utility/Log.h"
#include "game/component/ai/AIComponent.h"
#include "game/constant/ModelId.h"
#include "game/event/InGameEvents.h"
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
	    core::iface::IResourceManager& resourceManager,
	    core::base::EventBus& eventBus)
	    : m_factoryManager{ factoryManager }
	    , m_componentManager{ componentManager }
	    , m_resourceManager{ resourceManager }
	    , m_eventBus{ eventBus }
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
		// （死亡した同種の敵が返却したハンドルがあればそれを使い回す）
		int modelHandle{ acquireModelHandle(type, modelId) };
		auto meta{ m_resourceManager.getMetadata(modelId) };
		if (!meta.has_value())
		{
			core::log::info("ERROR: 敵メタデータが見つかりません: {}", std::string(modelId).c_str());
			throw std::runtime_error("敵メタデータの読み込みに失敗しました");
		}

		data::EnemyData enemyData{ data::EnemyData::fromMetadata(meta.value()) };
		enemyData.setPosition(position); // 位置は呼び出し側の指定が正
		const auto enemyId{ m_factoryManager.getEnemyFactory().create(modelHandle, enemyData) };

		// 敵種はスポーン時にしか分からないのでコンポーネントとして持たせる（推測させない）
		m_componentManager.add<component::EnemyTypeComponent>(enemyId, { type });

		// 追跡対象が設定されていれば反映する（召喚された敵も即プレイヤーを追う）
		if (m_target.getId() != 0 && m_componentManager.has<component::ai::AIComponent>(enemyId))
		{
			auto& ai = m_componentManager.get<component::ai::AIComponent>(enemyId);
			ai.m_targetEntity = m_target;
		}

		// スポーン演出（Enemy_Spawn）のトリガー。初期配置・ボスの召喚どちらもここを通る
		m_eventBus.publish(event::EnemySpawnedEvent{ enemyId, position });

		return enemyId;
	}

	void EnemySpawner::spawnStageEnemies()
	{
		const auto& stage{ m_resourceManager.getStageMetadata() };
		for (const auto& spawn : stage.m_spawns)
			this->spawn(constant::toEnemyType(spawn.m_type), spawn.m_position);
	}

	void EnemySpawner::returnEnemy(constant::EnemyType type, core::ecs::EntityId entityId, int modelHandle)
	{
		// 前の持ち主のアニメーションアタッチ状態を消し、次に使い回す際の二重アタッチを防ぐ
		m_resourceManager.detachAllAnimations(modelHandle);
		m_modelHandlePool[type].push_back(modelHandle);

		m_factoryManager.getEnemyFactory().remove(entityId);
	}

	int EnemySpawner::acquireModelHandle(constant::EnemyType type, std::string_view modelId)
	{
		auto& pool{ m_modelHandlePool[type] };
		if (!pool.empty())
		{
			int handle{ pool.back() };
			pool.pop_back();
			return handle;
		}

		int baseHandle{ m_resourceManager.loadModelById(modelId) };
		return m_resourceManager.duplicateModel(baseHandle);
	}
} // namespace game::factory
