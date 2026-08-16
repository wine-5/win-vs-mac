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
		 * @brief 円を描画する
		 * @param centerX 中心X座標
		 * @param centerY 中心Y座標
		 * @param radius 半径
		 * @param color 色（ARGB形式：0xAARRGGBB）
		 * @param isFilled 塗りつぶすかどうか
		 * @param thickness 線の太さ（塗りつぶしなしのときに有効）
		 */
		virtual void drawCircle(int centerX, int centerY, int radius, unsigned int color, bool isFilled, int thickness) = 0;

		/**
		 * @brief 三角形を描画する
		 * @param x1 頂点1のX座標
		 * @param y1 頂点1のY座標
		 * @param x2 頂点2のX座標
		 * @param y2 頂点2のY座標
		 * @param x3 頂点3のX座標
		 * @param y3 頂点3のY座標
		 * @param color 色（ARGB形式：0xAARRGGBB）
		 * @param isFilled 塗りつぶすかどうか
		 */
		virtual void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, unsigned int color, bool isFilled) = 0;

		/**
		 * @brief 直線をアンチエイリアス付きで描画する
		 *
		 * 矩形や三角形では表せない任意角度の線（ミニマップの床の輪郭など）に使う
		 * @param x1 始点のX座標
		 * @param y1 始点のY座標
		 * @param x2 終点のX座標
		 * @param y2 終点のY座標
		 * @param color 色（ARGB形式：0xAARRGGBB）
		 * @param thickness 線の太さ
		 */
		virtual void drawLine(int x1, int y1, int x2, int y2, unsigned int color, int thickness) = 0;

		/**
		 * @brief 角の丸い矩形をアンチエイリアス付きで描画する
		 *
		 * Windows 11（Fluent）のパネル・ボタンを再現するための基本形。
		 * 角丸の半径はパネルなら8px、ボタン等のコントロールなら4pxが実物の値。
		 * @param x X座標
		 * @param y Y座標
		 * @param width 幅
		 * @param height 高さ
		 * @param radius 角丸の半径
		 * @param color 色（ARGB形式：0xAARRGGBB）
		 * @param isFilled 塗りつぶすかどうか
		 * @param thickness 線の太さ（塗りつぶしなしのときに有効）
		 */
		virtual void drawRoundedBox(int x, int y, int width, int height, int radius, unsigned int color, bool isFilled, int thickness) = 0;

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

		/**
		 * @brief 以降の描画を矩形の内側だけに制限する
		 *
		 * ミニマップのように「枠からはみ出した中身を切り落とす」用途に使う。
		 * 使い終わったら必ず resetClipArea() で全画面へ戻すこと。
		 * 指定できるのは矩形のみで、円や角丸での切り抜きはできない
		 * @param x 左上X座標
		 * @param y 左上Y座標
		 * @param width 幅
		 * @param height 高さ
		 */
		virtual void setClipArea(int x, int y, int width, int height) = 0;

		/**
		 * @brief 描画範囲の制限を解除して全画面へ戻す
		 */
		virtual void resetClipArea() = 0;

		/**
		 * @brief 以降の描画位置をまとめてずらす
		 *
		 * HUDを丸ごと揺らす・落とすといった演出に使う。各Viewの座標計算に
		 * 手を入れず、描画を挟むだけで全体を動かせるようにするためのもの。
		 * 使い終わったら必ず resetDrawOffset() で戻すこと。
		 * @param x 横方向のずらし量（px）
		 * @param y 縦方向のずらし量（px）
		 */
		virtual void setDrawOffset(int x, int y) = 0;

		/**
		 * @brief 描画位置のずらしを解除する
		 */
		virtual void resetDrawOffset() = 0;
	};
} // namespace core::iface