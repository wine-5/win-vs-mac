#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"
#include "core/interface/IResourceManager.h"
#include "game/data/PlayerData.h"

namespace game::actor
{
	/**
	 * @brief Playerのセットアップを担当するクラス
	 */
	class Player
	{
	public:
		/**
		 * @brief Playerのコンストラクタ
		 * @param entityManager EntityManagerの参照
		 * @param componentManager ComponentManagerの参照
		 * @param resourceManager アニメーションハンドル取得用のIResourceManager
		 * @param modelHandle モデルハンドル
		 * @param playerData プレイヤーのデータ
		 */
		Player(core::ecs::EntityManager& entityManager,
			core::ecs::ComponentManager& componentManager,
			core::iface::IResourceManager& resourceManager,
			int modelHandle,
			const data::PlayerData& playerData);

		/**
		 * @brief PlayerのEntityIDを取得する
		 * @return EntityID
		 */
		core::ecs::EntityId getId() const noexcept;

	private:
		core::ecs::Entity m_entity;
	};
}