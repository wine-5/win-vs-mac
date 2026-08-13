#pragma once
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
	class PlayerFactory
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
		 * @param playerData Playerのデータ（装備ボーナス適用後）
		 * @param statBase 装備ボーナス適用前の能力値の控え
		 */
		void create(int modelHandle, const data::PlayerData& playerData,
		    const component::combat::PlayerStatBaseComponent& statBase);

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
} // namespace game::factory