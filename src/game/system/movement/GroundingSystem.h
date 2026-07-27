#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"

// 前方宣言（実体は .cpp でインクルード）
namespace game::component::movement
{
	struct TransformComponent;
	struct VelocityComponent;
} // namespace game::component::movement

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

		/**
		 * @brief 動く歩道に運ばれる速度を更新する
		 *
		 * 坂を滑る力が加速度なのに対し、こちらは一定速度で運ぶ。乗っている間は
		 * 常にこの速度が加わるので、歩き速度との大小がそのまま「逆走できる／できない」になる。
		 * 高さは接地処理が面に合わせて追従させるため、水平成分だけを扱う。
		 * @param velocity 対象のVelocityComponent
		 * @param conveyorVelocity 面が運ぶワールド速度
		 * @param deltaTime フレーム間の時間差
		 */
		void updateConveyor(component::movement::VelocityComponent& velocity,
		    const core::Vector3& conveyorVelocity, float deltaTime) const;

		/**
		 * @brief 面が乗っている者を運ぶワールド速度を求める
		 * @param surfaceId 面のEntityID
		 * @return 運ぶ速度。動く歩道でなければゼロベクトル
		 */
		core::Vector3 conveyorVelocityOf(core::ecs::EntityId surfaceId) const;

		core::ecs::ComponentManager& m_componentManager;
	};
} // namespace game::system::movement
