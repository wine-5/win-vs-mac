#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/utility/Vector3.h"
#include "game/stage/StageProp.h"
#include <memory>
#include <vector>

namespace game::factory
{
	/**
	 * @brief StageProp（床・壁などの配置物）の生成と寿命管理を担当
	 */
	class StagePropFactory
	{
	  public:
		/**
		 * @brief StagePropFactoryのコンストラクタ
		 * @param entityManager EntityManagerの参照
		 * @param componentManager ComponentManagerの参照
		 */
		StagePropFactory(
		    core::ecs::EntityManager& entityManager,
		    core::ecs::ComponentManager& componentManager);

		/**
		 * @brief 配置物を1つ生成する
		 * @param modelHandle モデルハンドル
		 * @param position 中心座標
		 * @param rotation 回転（ラジアン）
		 * @param scale モデルスケール（size ÷ baseSize）
		 * @param collision 当たり判定の役割（塞ぐ障害物か・歩ける面か・無しか）
		 * @param collisionSize 判定に使う実寸
		 * @return 生成した配置物のEntityId
		 */
		core::ecs::EntityId create(int modelHandle,
		    const core::Vector3& position,
		    const core::Vector3& rotation,
		    const core::Vector3& scale,
		    constant::PropCollision collision,
		    const core::Vector3& collisionSize);

	  private:
		core::ecs::EntityManager& m_entityManager;
		core::ecs::ComponentManager& m_componentManager;

		std::vector<std::unique_ptr<stage::StageProp>> m_props;
	};
} // namespace game::factory
