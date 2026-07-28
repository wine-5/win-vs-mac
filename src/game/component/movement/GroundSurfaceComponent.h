#pragma once
#include "core/utility/Vector3.h"

namespace game::component::movement
{
	/**
	 * @brief 歩ける面（床・通路・坂）であることを示すコンポーネント
	 *
	 * TransformComponent の位置・回転と合わせて「傾いた天面」を表す。
	 * GroundingSystem がこの面の高さを計算してプレイヤーや敵の足を乗せる。
	 * 軸並行(AABB)のColliderComponentでは傾いた坂を表現できないため分けている。
	 */
	struct GroundSurfaceComponent
	{
		core::Vector3 m_size{}; // 回転前の実寸（ワールドユニット）

		// 坂を滑り落ちる加速度。0なら滑らない普通の足場。
		// 傾きが急なほど強く働き、水平な面では効かない
		float m_slideAccel{ 0.0f };

		// 動く歩道として乗っている者を運ぶ速さ（ユニット/秒）。0なら運ばない。
		// 向きは面のローカル+Zをワールドへ回したもの。滑り落ちる向き（下り）ではなく
		// 面自身の向きなので、配置をY180度回せば運ぶ向きも反転する
		float m_conveyorSpeed{ 0.0f };
	};
} // namespace game::component::movement
