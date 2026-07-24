#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"

namespace game::system::movement
{
	/**
	 * @brief 歩ける面（床・通路・坂）の上にプレイヤーや敵の足を乗せるSystem
	 *
	 * 軸並行(AABB)の当たり判定では傾いた坂の面を表現できないため、
	 * GroundSurfaceComponent を持つ配置物の「傾いた天面」の高さを数式で求めて接地させる。
	 * 上へ持ち上げるだけで下へは引っ張らないので、障害物（Box）の上に立っている状態を壊さない。
	 */
	class GroundingSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief GroundingSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 */
		GroundingSystem(core::ecs::ComponentManager& componentManager);

		/**
		 * @brief 各エンティティを足元の面へ接地させる
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		/**
		 * @brief 傾いた天面の、指定XZ位置での高さを求める
		 * @param surfaceId 面のEntityID
		 * @param x ワールドX座標
		 * @param z ワールドZ座標
		 * @param outHeight 求まった高さの格納先
		 * @return XZが面の範囲内で高さが求まった場合true
		 */
		bool surfaceHeightAt(core::ecs::EntityId surfaceId, float x, float z, float& outHeight) const;

		core::ecs::ComponentManager& m_componentManager;
	};
} // namespace game::system::movement
