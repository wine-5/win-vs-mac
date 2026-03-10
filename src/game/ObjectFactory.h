#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "game/actor/Player.h"
#include "core/interface/IResourceManager.h"
#include "game/data/PlayerData.h"
#include <memory>

namespace game
{
	// TODO: このままだとオブジェクトの種類が増えるたびにクラス内が肥大化するため、
	// IFactoryなどの純粋仮想クラスなどを作り責務分離をすること
	/**
	 * @brief ゲームオブジェクトの生成・破棄を担当するクラス
	 */
	class ObjectFactory
	{
	public:
		/**
		 * @brief ObjectFactoryのコンストラクタ
		 * @param entityManager EntityManagerの参照
		 * @param componentManager ComponentManagerの参照
		 * @param resourceManager IResourceManagerの参照
		 */
		ObjectFactory(core::ecs::EntityManager& entityManager, core::ecs::ComponentManager& componentManager,core::iface::IResourceManager& resourceManager);
		
		/**
		 * @brief Playerオブジェクトを初期化する
		 * @param playerModelhandle Playerのモデルハンドル
		 * @param playerData Playerのデータ
		 */
		void init(int playerModelhandle, const data::PlayerData& playerData);

		/**
		 * @brief 生成したPlayerオブジェクトを取得する
		 * @return Playerオブジェクトの参照
		 */
		[[nodiscard]] actor::Player& getPlayer() const;

	private:
		core::ecs::EntityManager& m_entityManager;
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IResourceManager& m_resourceManager;

		std::unique_ptr<actor::Player> m_player; // init()で生成タイミングを遅らせるためunique_ptrで保持
	};
}