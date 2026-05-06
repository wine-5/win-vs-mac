#pragma once

namespace core::constant
{
    /**
     * @brief UI関連の定数
     */
    namespace ui
    {
        // フォントサイズ
        constexpr int FONT_SIZE_SMALL = 12;
        constexpr int FONT_SIZE_NORMAL = 16;
        constexpr int FONT_SIZE_LARGE = 24;
        constexpr int FONT_SIZE_TITLE = 32;

        // デフォルトのフォントサイズ
        constexpr int DEFAULT_FONT_SIZE = FONT_SIZE_NORMAL;

        // ブレンドモード（DxLib の DX_BLENDMODE_ * と対応）
        constexpr int BLEND_MODE_NONE = 0;
        constexpr int BLEND_MODE_ALPHA = 1;
    }
}