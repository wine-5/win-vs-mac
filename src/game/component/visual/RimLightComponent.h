#pragma once
#include "core/utility/Color.h"

namespace game::component::visual
{
	/**
	 * @brief 輪郭を光らせる（リムライト）対象であることを示すコンポーネント
	 *
	 * 明るい床の上では暗いキャラクターが背景へ溶けて見失いやすい。
	 * 輪郭を光らせることで、見栄えだけでなく「自機がどこにいるか」の把握を助ける。
	 *
	 * 付けたEntityだけが対象。本体が消えているとき（RenderComponentの m_isVisible が
	 * false のとき）は輪郭も描かれない。
	 */
	struct RimLightComponent
	{
		// 本体を膨らませる割合。
		//
		// モデルの原点は足元にあるため、膨らませると上方向にもズレる。
		// 大きくしすぎると輪郭ではなく「一回り大きい分身」が背後に立っているように
		// 見えるので、数%に留めること（0.15 だと明らかに分身に見える）
		float m_thickness{ 0.02f };

		// 輪郭の色（ARGB。アルファは見ない）
		unsigned int m_color{ core::utility::Color::HUD_CHARGE_CYAN };
	};
} // namespace game::component::visual
