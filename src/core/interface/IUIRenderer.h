#pragma once

namespace core::iface
{
    /**
     * @brief UI描画のインターフェース
     * Game層がInfrastructure層（DxLib）に直接依存しないための抽象化
     */
    class IUIRenderer
    {
    public:
        virtual ~IUIRenderer() = default;

        /**
         * @brief 矩形を描画する
         * @param x X座標
         * @param y Y座標
         * @param width 幅
         * @param height 高さ
         * @param color 色（ARGB形式：0xAARRGGBB）
         * @param isFilled 塗りつぶすかどうか
         */
        virtual void drawBox(int x, int y, int width, int height, unsigned int color, bool isFilled) = 0;

        /**
         * @brief テキストを描画する
         * @param x X座標
         * @param y Y座標
         * @param text テキスト
         * @param color 色（ARGB形式：0xAARRGGBB）
         */
        virtual void drawText(int x, int y, const char* text, unsigned int color) = 0;

        /**
         * @brief テキストの描画幅を取得する
         * @param text テキスト
         * @return 描画幅（ピクセル）
         */
        virtual int getTextWidth(const char* text) const = 0;
    };
}