#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/Vector3.h"

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
		 * @param modelHandle モデルハンドル
		 * @param colliderSize コライダーのサイズ
		 * @param colliderOffset コライダーのオフセット
		 */
		Player(core::ecs::EntityManager& entityManager,
			core::ecs::ComponentManager& componentManager,
			int modelHandle,
			core::Vector3 colliderSize,
			core::Vector3 colliderOffset);

		/**
		 * @brief PlayerのEntityIDを取得する
		 * @return EntityID
		 */
		core::ecs::EntityId getId() const noexcept;

	private:
		core::ecs::Entity m_entity;
	};
}