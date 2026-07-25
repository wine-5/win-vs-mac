#pragma once
#include "core/utility/Vector3.h"

namespace game::component::visual
{
	/**
	 * @brief エンティティに点光源を持たせるコンポーネント
	 *
	 * LightSystem がこの定義から実際の光源を作り、毎フレーム位置を追従させる。
	 * プレイヤーに付ければ「自機が周囲を照らす」、ステージに置けば「光の溜まり」になる。
	 *
	 * @note DxLibのライトはワールド全体に効くため、「特定のオブジェクトだけを照らす」
	 *       指定はできない。あくまで「どこに光源を置くか」で演出する。
	 */
	struct LightComponent
	{
		core::Vector3 m_offset{}; // エンティティ位置からのずれ（頭上に置く等）
		float m_range{ 1000.0f }; // 光の届く距離
		int m_r{ 255 };
		int m_g{ 255 };
		int m_b{ 255 };

		int m_lightHandle{ -1 }; // LightSystemが管理する実体（-1は未生成）
	};
} // namespace game::component::visual
