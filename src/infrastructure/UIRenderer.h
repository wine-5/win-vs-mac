#pragma once
#include "core/interface/IUIRenderer.h"
#include <string>
#include <map>
#include <utility>

namespace infrastructure
{
    /**
     * @brief UI描画を担当するクラス（DxLib実装）
     */
    class UIRenderer : public core::iface::IUIRenderer
    {
    public:
        /**
         * @brief コンストラクタ
         * @param defaultFontName デフォルトで使用するフォント名（空文字でDxLib組み込みフォント）
         */
        explicit UIRenderer(std::string defaultFontName = {});

        ~UIRenderer();

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
		 * @brief 円を描画する
		 * @param centerX 中心X座標
		 * @param centerY 中心Y座標
		 * @param radius 半径
		 * @param color 色（ARGB形式：0xAARRGGBB）
		 * @param isFilled 塗りつぶすかどうか
		 * @param thickness 線の太さ（塗りつぶしなしのときに有効）
		 */
		void drawCircle(int centerX, int centerY, int radius, unsigned int color, bool isFilled, int thickness) override;

		/**
         * @brief テキストを描画する
         * @param x X座標
         * @param y Y座標
         * @param text テキスト
         * @param color 色（ARGB形式：0xAARRGGBB）
         */
        void drawText(int x, int y, const char *text, unsigned int color, int fontSize) override;

        /**
         * @brief テキストの描画幅を取得する
         * @param text テキスト
         * @param fontSize フォントサイズ
         * @return 描画幅（ピクセル）
         */
        [[nodiscard]] int getTextWidth(const char *text, int fontSize) const override;

        /**
         * @brief 描画ブレンドモードを設定する
         * @param blendMode ブレンドモード
         * @param alpha アルファ値（0〜255）
         */
        void setBlendMode(int blendMode, int alpha) override;

        /**
         * @brief 描画ブレンドモードをリセットする
         */
        void resetBlendMode() override;

        /**
         * @brief 描画に使うフォントを設定する
         * @param fontName フォント名
         */
        void setFont(const char *fontName) override;

        /**
         * @brief フォントをデフォルトに戻す
         */
        void resetFont() override;

        /**
         * @brief 画像を指定矩形に拡大縮小して描画する
         * @param handle IResourceManager::loadImageById で取得したハンドル
         * @param x      左上 X 座標
         * @param y      左上 Y 座標
         * @param width  描画幅
         * @param height 描画高さ
         */
        void drawImage(int handle, int x, int y, int width, int height) override;

    private:
        std::string m_defaultFontName{};
        std::string m_currentFontName{};
        // mutable: getTextWidth はフォントハンドルを遅延生成してキャッシュする
        // キャッシュは内部実装の詳細であり論理的な const 性を損なわないため mutable としている
        mutable std::map<std::pair<std::string, int>, int> m_fontHandles{};
    };
} // namespace infrastructure