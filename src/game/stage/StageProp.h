#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"

namespace game::stage
{
	/**
	 * @brief ステージの配置物（床・壁・柱など）1つ分のオブジェクト
	 *
	 * 立方体モデルをXYZに引き伸ばして床・通路・壁・柱を賄うグレーボックス方式。
	 * 位置・回転（ラジアン）・スケール・コライダーは解決済みの値を受け取り、
	 * ここではEntityへのComponent組み立てのみを行う（Ground.hと同じ責務分担）。
	 */
	class StageProp
	{
	  public:
		/**
		 * @brief StagePropのコンストラクタ
		 * @param entityManager EntityManagerの参照
		 * @param componentManager ComponentManagerの参照
		 * @param modelHandle モデルハンドル
		 * @param position 中心座標
		 * @param rotation 回転（ラジアン）
		 * @param scale モデルスケール（size ÷ baseSize）
		 * @param colliderSize コライダーの実寸。全成分0ならコライダーを付けない
		 */
		StageProp(core::ecs::EntityManager& entityManager,
		    core::ecs::ComponentManager& componentManager,
		    int modelHandle,
		    const core::Vector3& position,
		    const core::Vector3& rotation,
		    const core::Vector3& scale,
		    const core::Vector3& colliderSize);

		/**
		 * @brief StagePropのEntityIDを取得する
		 * @return EntityID
		 */
		core::ecs::EntityId getId() const noexcept;

	  private:
		core::ecs::Entity m_entity;
	};
} // namespace game::stage
