#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

// 前方宣言（実体は .cpp でインクルード）
namespace game::component::movement
{
	struct TransformComponent;
	struct VelocityComponent;
} // namespace game::component::movement

namespace game::system::combat
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

		/**
		 * @brief 縦方向（上下）の押し出しを解決する
		 *
		 * riderが上なら地面に乗せ（着地・死亡バウンド処理を含む）、下なら天井として押し戻す。
		 * @param riderId 乗る側のEntityID
		 * @param riderTransform 乗る側のTransform
		 * @param riderVelocity 乗る側のVelocity
		 * @param overlapY Y軸のめり込み量（正）
		 * @param deltaY 乗る側中心 − 相手中心 のY成分（符号で上下を判定）
		 */
		void resolveVertical(core::ecs::EntityId riderId,
		    component::movement::TransformComponent& riderTransform,
		    component::movement::VelocityComponent& riderVelocity,
		    float overlapY, float deltaY);

		core::ecs::ComponentManager& m_componentManager;
    };
} // namespace game::system::combat