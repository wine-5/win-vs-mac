#pragma once
#include "core/constant/UI.h"

namespace core::iface
{
    /**
     * @brief UI描画のインターフェース
     * Game層がInfrastructure層（DxLib）に直接依存しないための抽象化
     * そのため基本的にはDxLibを用いた描画関数を書くこと
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
         * @param fontSize フォントサイズ
         */
        virtual void drawText(int x, int y, const char* text, unsigned int color, int fontSize) = 0;

        /**
         * @brief テキストの描画幅を取得する
         * @param text テキスト
         * @param fontSize フォントサイズ
         * @return 描画幅（ピクセル）
         */
        virtual int getTextWidth(const char* text, int fontSize) const = 0;

        /**
         * @brief 描画ブレンドモードを設定する
         * @param blendMode ブレンドモード（DX_BLENDMODE_ALPHA 等）
         * @param alpha アルファ値（0〜255）
         */
        virtual void setBlendMode(int blendMode, int alpha) = 0;

        /**
         * @brief 描画ブレンドモードをリセットする（通常描画に戻す）
         */
        virtual void resetBlendMode() = 0;

        /**
         * @brief 描画に使うフォントを設定する
         * @param fontName フォント名
         */
        virtual void setFont(const char* fontName) = 0;

        /**
         * @brief フォントをデフォルトに戻す
         */
        virtual void resetFont() = 0;

        /**
         * @brief 画像を指定矩形に拡大縮小して描画する
         * @param handle IResourceManager::loadImageById で取得したハンドル
         * @param x      左上 X 座標
         * @param y      左上 Y 座標
         * @param width  描画幅
         * @param height 描画高さ
         */
        virtual void drawImage(int handle, int x, int y, int width, int height) = 0;
    };
} // namespace core::iface