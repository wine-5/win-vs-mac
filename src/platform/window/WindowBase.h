#pragma once

#include <windows.h>
#include <string>

namespace platform::window
{
    /**
     * @class WindowBase
     * @brief Win32 ウィンドウの基底クラス
     */
    class WindowBase
    {
    public:
        /**
        * @brief コンストラクタ
        * @param className ウィンドウクラス名
        * @param title ウィンドウタイトル
        * @param x ウィンドウの左上角 X 座標
        * @param y ウィンドウの左上角 Y 座標
        * @param width ウィンドウの幅
        * @param height ウィンドウの高さ
        */
        WindowBase(const wchar_t* className, const wchar_t* title,
            int x, int y, int width, int height) noexcept;

        virtual ~WindowBase() noexcept;

        /**
         * @brief ウィンドウを作成する
         * @return 成功時 true
         */
        bool create() noexcept;

        /**
         * @brief ウィンドウを破棄する
         */
        void destory() noexcept;

        /**
         * @brief ウィンドウ非表示にする
         */
        void hide() noexcept;

        /**
         * @brief ウィンドウを最前面に持ってくる
         */
        void bringToFront() noexcept;

        /**
         * @brief ウィンドウハンドルを取得
         * @return ウィンドウの HWND
         */
        [[nodiscard]] HWND getHwnd() const noexcept;

        /**
        * @brief ウィンドウが作成されているか
        * @return 作成済みなら true
        */
        [[nodiscard]] bool isCreated() const noexcept;

    protected:
        /**
         * @brief コントロール作成時のコールバック
         * @param hwnd ウィンドウハンドル
         */
        virtual void onCreateControls(HWND hwnd) {}

        /**
         * @brief ウィンドウメッセージハンドラ
         * @param msg ウィンドウメッセージ
         * @param wParam メッセージパラメータ
         * @param lParam メッセージパラメータ
         * @return メッセージ処理結果
         */
        virtual LRESULT onMessage(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

        std::wstring m_className{};
        std::wstring m_title{};
        int m_x{};
        int m_y{};
        int m_width{};
        int m_height{};
        HWND m_hwnd{};

    private:
        static LRESULT CALLBACK staticWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    }; 

}