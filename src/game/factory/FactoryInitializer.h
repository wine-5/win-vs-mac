#pragma once
#include "FactoryManager.h"
#include "core/interface/IResourceManager.h"
#include "game/data/PlayerData.h"
#include "game/data/GroundData.h"
#include "game/constant/ModelId.h"
#include <cassert>

namespace game::factory
{
	/**
	 * @brief FactoryManagerを使ったエンティティの初期化処理のみを担当
	 * @note オブジェクトの種類が増えた場合は、JSONからの一括読み込みや
	 *       クラス分離への切り替えを検討すること。
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

	private:
		FactoryManager& m_factoryManager;
		core::iface::IResourceManager& m_resourceManager;
	};
}