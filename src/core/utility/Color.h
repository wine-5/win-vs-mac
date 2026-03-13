#pragma once

namespace core::utility
{
    /**
     * @brief 色を扱うユーティリティクラス
     */
    class Color
    {
    public:
        /**
         * @brief RGB値から色を作成する（Alpha=255）
         * @param r 赤成分（0-255）
         * @param g 緑成分（0-255）
         * @param b 青成分（0-255）
         * @return ARGB形式の色（0xAARRGGBB）
         */
        static constexpr unsigned int rgb(int r, int g, int b)
        {
            return 0xFF000000 | (r << 16) | (g << 8) | b;
        }

        /**
         * @brief ARGB値から色を作成する
         * @param a アルファ成分（0-255、255=不透明）
         * @param r 赤成分（0-255）
         * @param g 緑成分（0-255）
         * @param b 青成分（0-255）
         * @return ARGB形式の色（0xAARRGGBB）
         */
        static constexpr unsigned int argb(int a, int r, int g, int b)
        {
            return (a << 24) | (r << 16) | (g << 8) | b;
        }

        // ========== よく使う色を定義 ==========

        static constexpr unsigned int WHITE = 0xFFFFFFFF;
        static constexpr unsigned int BLACK = 0xFF000000;
        static constexpr unsigned int RED = 0xFFFF0000;
        static constexpr unsigned int GREEN = 0xFF00FF00;
        static constexpr unsigned int BLUE = 0xFF0000FF;
        static constexpr unsigned int YELLOW = 0xFFFFFF00;
        static constexpr unsigned int CYAN = 0xFF00FFFF;
        static constexpr unsigned int MAGENTA = 0xFFFF00FF;
        static constexpr unsigned int GRAY = 0xFF808080;
        static constexpr unsigned int DARK_GRAY = 0xFF404040;
        static constexpr unsigned int LIGHT_GRAY = 0xFFC0C0C0;
    };
}