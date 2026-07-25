#pragma once
#include "core/utility/Vector3.h"

namespace game::component::combat
{
	/**
	 * @brief 当たり判定の形状のデータを保持するコンポーネント
	 */
	struct ColliderComponent
	{
		// JSONから設定されるフィールド
		core::Vector3 m_size;      // 当たり判定のサイズ（幅、高さ、奥行き)
		core::Vector3 m_offset;    // 中心からのオフセット
		float m_rotationY{ 0.0f }; // Y軸まわりの向き（ラジアン）。斜めに置いた配置物で使う
	};
} // namespace game::component::combat