#pragma once

namespace core::iface
{   
    /**
     * @brief 画面サイズを提供するインターフェース
     */
    class IScreen
    {
    public:
        virtual ~IScreen() = default;

        /**
         * @brief 画面幅を取得
         * @return 画面の幅（ピクセル）
         */
        virtual int getWidth() const noexcept = 0;

        /**
         * @brief 画面高さを取得
         * @return 画面の高さ（ピクセル）
         */
        virtual int getHeight() const noexcept = 0;

        /**
         * @brief DxLib メインウィンドウのネイティブハンドルを取得する
         * @details Platform 層が HWND として利用するための void* ラッパー
         * @return ネイティブウィンドウハンドル（HWND を void* にキャストしたもの）
         */
        virtual void* getNativeWindowHandle() const noexcept = 0;

		/**
		 * @brief 画面クリア時の背景色を設定する
		 * @param r 赤成分（0-255）
		 * @param g 緑成分（0-255）
		 * @param b 青成分（0-255）
		 */
		virtual void setBackgroundColor(int r, int g, int b) noexcept = 0;

		/**
		 * @brief 距離フォグを設定する
		 *
		 * 背景と同じ色のフォグを掛けると、遠くの床が闇へ溶けて「虚無に浮かぶ」画になる。
		 * @param enable 有効にするならtrue
		 * @param r フォグ色の赤成分（0-255）
		 * @param g フォグ色の緑成分（0-255）
		 * @param b フォグ色の青成分（0-255）
		 * @param startDistance 掛かり始める距離（ワールドユニット）
		 * @param endDistance 完全にフォグ色になる距離（ワールドユニット）
		 */
		virtual void setFog(bool enable, int r, int g, int b, float startDistance, float endDistance) noexcept = 0;
	};
} // namespace core::iface