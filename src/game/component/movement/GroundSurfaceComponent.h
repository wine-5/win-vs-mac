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
	};
} // namespace game::component::movement
