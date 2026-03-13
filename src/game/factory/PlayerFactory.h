#pragma once
#include "IFactory.h"
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/interface/IResourceManager.h"
#include "game/actor/Player.h"
#include "game/data/PlayerData.h"
#include <memory>

namespace game::factory
{
	/**
	 * @brief Playerオブジェクトの生成と寿命管理を担当
	 */
	class PlayerFactory : public IFactory
	{
	public:
		/**
		 * @brief PlayerFactoryのコンストラクタ
		 * @param entityManager EntityManagerの参照
		 * @param componentManager ComponentManagerの参照
		 * @param resourceManager IResourceManagerの参照
		 */
		PlayerFactory(
			core::ecs::EntityManager& entityManager,
			core::ecs::ComponentManager& componentManager,
			core::iface::IResourceManager& resourceManager);
		
		/**
		 * @brief Playerオブジェクトを生成する
		 * @param modelHandle モデルハンドル
		 * @param playerData Playerのデータ
		 */
		void create(int modelHandle, const data::PlayerData& playerData);

		/**
		 * @brief 生成したPlayerオブジェクトを取得する
		 * @return Playerオブジェクトの参照
		 */
		actor::Player& getPlayer() const;

	private:
		core::ecs::EntityManager& m_entityManager;
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IResourceManager& m_resourceManager;

		std::unique_ptr<actor::Player> m_player{};
	};
}