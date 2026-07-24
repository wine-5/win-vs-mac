#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"

// 前方宣言（実体は .cpp でインクルード）
namespace game::component::movement
{
	struct VelocityComponent;
}

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
		 * @param outNormal 天面の法線の格納先（滑り方向の算出に使う）
		 * @return XZが面の範囲内で高さが求まった場合true
		 */
		bool surfaceHeightAt(core::ecs::EntityId surfaceId, float x, float z,
		    float& outHeight, core::Vector3& outNormal) const;

		/**
		 * @brief 坂を滑り落ちる速度を更新する
		 *
		 * 法線の水平成分がそのまま「坂を下る向き」になる。傾きが急なほど強く加速し、
		 * 水平な面では働かない。入力速度とは別枠で持つため、歩き速度との大小が
		 * そのまま「登れる／登れない」になる。
		 * @param velocity 対象のVelocityComponent
		 * @param normal 接地している面の法線
		 * @param slideAccel 面の滑り加速度（0なら減衰させて止める）
		 * @param deltaTime フレーム間の時間差
		 */
		void updateSlide(component::movement::VelocityComponent& velocity,
		    const core::Vector3& normal, float slideAccel, float deltaTime) const;

		core::ecs::ComponentManager& m_componentManager;
	};
} // namespace game::system::movement
