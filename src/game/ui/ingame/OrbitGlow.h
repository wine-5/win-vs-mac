#pragma once
#include "core/interface/IUIRenderer.h"

namespace game::ui::ingame::orbit_glow
{
	/// @brief 先頭の粒の半径（1080p基準）。呼び出し側で画面サイズに合わせて拡縮する
	inline constexpr int DOT_RADIUS{ 3 };

	/// @brief スロットごとに位相をずらす量。同じ量ずつずらして同期させない
	inline constexpr float PHASE_PER_SLOT{ 0.33f };

	/**
	 * @brief 矩形の縁に沿って光の粒を周回させる
	 *
	 * 静止したUIは死んで見える。効果が乗っているマスだけを常に動かすことで、
	 * 「これは今も走っているプログラムだ」という見え方にする。
	 * 一周の速さ・粒の数・色はここで一元管理し、画面ごとに動きの印象がずれないようにする。
	 * @param uiRenderer UI描画のインターフェース
	 * @param x 矩形左上のX座標
	 * @param y 矩形左上のY座標
	 * @param width 矩形の幅
	 * @param height 矩形の高さ
	 * @param elapsedSeconds 演出の基準時刻からの経過秒数
	 * @param phaseOffset 周回位相のずらし量（0.0〜1.0）
	 * @param dotRadius 先頭の粒の半径（画面サイズに合わせた値）
	 * @param color 粒の色。マスの縁と揃えると、1つのマスが主張する色が1つで済む
	 */
	void draw(core::iface::IUIRenderer& uiRenderer, int x, int y, int width, int height,
	    float elapsedSeconds, float phaseOffset, int dotRadius, unsigned int color);
} // namespace game::ui::ingame::orbit_glow
