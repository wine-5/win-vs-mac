// src/infrastructure/graphics/Screen.h
#pragma once
#include "core/interface/IScreen.h"

namespace infrastructure::graphics
{
    /**
     * @brief 画面情報を提供するクラス
     * SetGraphMode()で設定した画面サイズを保持する
     */
    class Screen : public core::iface::IScreen
    {
    public:
        /**
         * @brief 画面サイズを指定して初期化
         * @param width 画面幅
         * @param height 画面高さ
         */
        Screen(int width, int height);

        /**
         * @brief 画面幅を取得
         * @return 画面の幅（ピクセル）
         */
        int getWidth() const noexcept override;

        /**
         * @brief 画面高さを取得
         * @return 画面の高さ（ピクセル）
         */
        int getHeight() const noexcept override;

        /**
         * @brief DxLib メインウィンドウのネイティブハンドルを取得する
         * @return ネイティブウィンドウハンドル（HWND を void* にキャストしたもの）
         */
        void* getNativeWindowHandle() const noexcept override;

		/**
		 * @brief 画面クリア時の背景色を設定する
		 * @param r 赤成分（0-255）
		 * @param g 緑成分（0-255）
		 * @param b 青成分（0-255）
		 */
		void setBackgroundColor(int r, int g, int b) noexcept override;

		/**
		 * @brief 距離フォグを設定する
		 * @param enable 有効にするならtrue
		 * @param r フォグ色の赤成分（0-255）
		 * @param g フォグ色の緑成分（0-255）
		 * @param b フォグ色の青成分（0-255）
		 * @param startDistance 掛かり始める距離（ワールドユニット）
		 * @param endDistance 完全にフォグ色になる距離（ワールドユニット）
		 */
		void setFog(bool enable, int r, int g, int b, float startDistance, float endDistance) noexcept override;

	  private:
        int m_width;
        int m_height;
    };
} // namespace infrastructure::graphics