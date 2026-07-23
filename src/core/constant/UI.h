#pragma once

namespace core::constant
{
    /**
     * @brief UI関連の定数
     */
    namespace ui
    {
        // フォントサイズの比率（1080pを基準に計算）
        // Windowのサイズに依存して大きさを変更したいため
        constexpr float FONT_SIZE_EXTRA_SMALL_RATIO = 0.0185f;  // 20 / 1080
        constexpr float FONT_SIZE_SMALL_RATIO = 0.0259f;        // 28 / 1080
        constexpr float FONT_SIZE_NORMAL_RATIO = 0.0324f;       // 35 / 1080
        constexpr float FONT_SIZE_LARGE_RATIO = 0.0519f;        // 56 / 1080
        constexpr float FONT_SIZE_TITLE_RATIO = 0.0778f;        // 84 / 1080
        constexpr float FONT_SIZE_CLOCK_RATIO = 0.1167f;        // 126 / 1080

        // デフォルトのフォントサイズ比率
        constexpr float DEFAULT_FONT_SIZE_RATIO = FONT_SIZE_NORMAL_RATIO;

        // ブレンドモード（DxLib の DX_BLENDMODE_ * と対応）
        constexpr int BLEND_MODE_NONE = 0;
        constexpr int BLEND_MODE_ALPHA = 1;
	} // namespace ui
} // namespace core::constant