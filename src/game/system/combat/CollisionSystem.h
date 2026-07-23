#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

namespace game::system
{
    /**
     * @brief ColliderComponentを持つ全エンティティ間の衝突検出と押し返しを行うSystem
     */
    class CollisionSystem : public core::ecs::ISystem
    {
    public:
		/**
		 * @brief CollisionSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 */
		CollisionSystem(core::ecs::ComponentManager& componentManager);

		/**
		 * @brief 全Entity間の衝突検出と押し返しを行う
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	private:
		/**
		 * @brief 2つのEntity間でAABB衝突判定を行う
		 * @param a EntityID A
		 * @param b EntityID B
		 * @return 衝突している場合true
		 */
		bool isColliding(core::ecs::EntityId a, core::ecs::EntityId b) const;

		/**
		 * @brief 衝突を解決し、押し返し処理を行う
		 * @param a EntityID A
		 * @param b EntityID B
		 */
		void resolveCollision(core::ecs::EntityId a, core::ecs::EntityId b);

		core::ecs::ComponentManager& m_componentManager;
    };
} // namespace game::system