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
        constexpr float FONT_SIZE_SMALL_RATIO = 0.0148f;    // 16 / 1080
        constexpr float FONT_SIZE_NORMAL_RATIO = 0.0185f;   // 20 / 1080
        constexpr float FONT_SIZE_LARGE_RATIO = 0.0296f;    // 32 / 1080
        constexpr float FONT_SIZE_TITLE_RATIO = 0.0444f;    // 48 / 1080
        constexpr float FONT_SIZE_CLOCK_RATIO = 0.0667f;    // 72 / 1080

        // デフォルトのフォントサイズ比率
        constexpr float DEFAULT_FONT_SIZE_RATIO = FONT_SIZE_NORMAL_RATIO;

        // 後方互換性のためのピクセル値定数（デフォルト値用）
        constexpr int FONT_SIZE_SMALL = 16;
        constexpr int FONT_SIZE_NORMAL = 20;
        constexpr int FONT_SIZE_LARGE = 32;
        constexpr int FONT_SIZE_TITLE = 48;
        constexpr int FONT_SIZE_CLOCK = 72;
        constexpr int DEFAULT_FONT_SIZE = FONT_SIZE_NORMAL;

        // ブレンドモード（DxLib の DX_BLENDMODE_ * と対応）
        constexpr int BLEND_MODE_NONE = 0;
        constexpr int BLEND_MODE_ALPHA = 1;
    }
}