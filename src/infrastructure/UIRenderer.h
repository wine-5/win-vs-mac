#pragma once
#include "core/interface/IUIRenderer.h"

namespace infrastructure
{
    /**
     * @brief UI描画を担当するクラス（DxLib実装）
     */
    class UIRenderer : public core::iface::IUIRenderer
    {
    public:
        /**
         * @brief 矩形を描画する
         * @param x X座標
         * @param y Y座標
         * @param width 幅
         * @param height 高さ
         * @param color 色（ARGB形式：0xAARRGGBB）
         * @param isFilled 塗りつぶすかどうか
         */
        void drawBox(int x, int y, int width, int height, unsigned int color, bool isFilled) override;

        /**
         * @brief テキストを描画する
         * @param x X座標
         * @param y Y座標
         * @param text テキスト
         * @param color 色（ARGB形式：0xAARRGGBB）
         */
        void drawText(int x, int y, const char* text, unsigned int color,int fontSize) override;

        /**
         * @brief テキストの描画幅を取得する
         * @param text テキスト
         * @return 描画幅（ピクセル）
         */
        [[nodiscard]] int getTextWidth(const char* text) const override;
    };
}