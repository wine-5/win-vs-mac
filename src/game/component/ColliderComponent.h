#pragma once
#include "core/Vector3.h"
#include "game/constant/CollisionTag.h"

namespace game::component
{
	/**
	 * @brief 当たり判定の形状のデータを保持するコンポーネント
	 */
	struct ColliderComponent
	{
		core::Vector3 m_size; // 当たり判定のサイズ（幅、高さ、奥行き)
		core::Vector3 m_offset; // 中心からのオフセット
		constant::CollisionTag m_tag{constant::CollisionTag::None}; // オブジェクト識別用のタグ
	};
}