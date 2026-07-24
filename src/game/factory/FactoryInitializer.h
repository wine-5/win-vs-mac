#pragma once
#include "FactoryManager.h"
#include "core/interface/IResourceManager.h"
#include "game/data/PlayerData.h"
#include "game/data/GroundData.h"
#include <cassert>

namespace game::factory
{
	/**
	 * @brief FactoryManagerを使ったエンティティの初期化処理のみを担当
	 * @note 敵の生成は EnemySpawner に委譲している（初期スポーン・召喚で共有するため）。
	 */
	class FactoryInitializer
	{
	public:
		/**
		* @brief コンストラクタ
		* @param factoryManager FactoryManagerの参照
		* @param resourceManager IResourceManagerの参照
		*/
		FactoryInitializer(
			FactoryManager& factoryManager,
			core::iface::IResourceManager& resourceManager);

		/**
		 * @brief Playerを初期化
		 * @param playerData Playerのデータ
		 */
		void initializePlayer(const data::PlayerData& playerData);

		/**
		 * @brief Groundを初期化
		 * @return 生成したGroundのEntityId
		 */
		core::ecs::EntityId initializeGround();

		/**
		 * @brief stageData.jsonのprops[]から配置物（床・壁など）を一括生成する
		 *
		 * 各propのtypeをstageCatalogで解決し、size÷baseSizeをモデルスケール、
		 * 回転（度）をラジアンへ変換して生成する。
		 */
		void initializeProps();

	  private:
		FactoryManager& m_factoryManager;
		core::iface::IResourceManager& m_resourceManager;
	};
} // namespace game::factory