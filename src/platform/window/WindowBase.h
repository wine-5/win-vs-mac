#pragma once

#include <windows.h>
#include <string>
#include <functional>

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
         * @param ownerHwnd オーナーウィンドウ (nullptr でオーナーなし)
         * @return 成功時 true
         */
        bool create(HWND ownerHwnd = nullptr) noexcept;

        /**
         * @brief ウィンドウを破棄する
         */
        void destroy() noexcept;

        /**
         * @brief ウィンドウを表示する
         */
        void show() noexcept;

        /**
         * @brief ウィンドウを非表示にする
         */
        void hide() noexcept;

        /**
         * @brief ウィンドウを最前面に持ってくる
         */
        void bringToFront() noexcept;

        /**
         * @brief 最小化ボタン押下時のコールバックを設定する
         * @details 設定するとデフォルトの最小化を抑制してコールバックを呼ぶ
         * @param callback 最小化時に呼ぶコールバック
         */
        void setOnMinimize(std::function<void()> callback) noexcept;

        /**
         * @brief ウィンドウ閉じボタン（×）押下時のコールバックを設定する
         * @details 設定するとデフォルトの閉じるを抑制してコールバックを呼ぶ
         * @param callback ウィンドウ閉時に呼ぶコールバック
         */
        void setOnClose(std::function<void()> callback) noexcept;

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

        /**
         * @brief ウィンドウの幅を取得
         * @return ウィンドウの幅（ピクセル）
         */
        [[nodiscard]] int getWidth() const noexcept;

        /**
         * @brief ウィンドウの高さを取得
         * @return ウィンドウの高さ（ピクセル）
         */
        [[nodiscard]] int getHeight() const noexcept;

        /**
         * @brief ウィンドウの透明度を設定する
         * @param alpha 不透明度（0=完全透明, 255=完全不透明）
         */
        void setAlpha(BYTE alpha) noexcept;

        /**
         * @brief ウィンドウアイコンを設定する
         * @param iconPath .ico ファイルのパス（実行ディレクトリからの相対パス）
         */
        void setIcon(HWND hwnd, const wchar_t* iconPath) noexcept;

        /**
         * @brief コントロール作成時のコールバック
         * @param hwnd ウィンドウハンドル
         */
        virtual void onCreateControls(HWND hwnd) {}

        /**
         * @brief ウィンドウメッセージハンドラ
         * @param hwnd ウィンドウハンドル
         * @param msg ウィンドウメッセージ
         * @param wParam メッセージパラメータ
         * @param lParam メッセージパラメータ
         * @return メッセージ処理結果
         */
        virtual LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

        std::function<void()> m_onMinimize{};
        std::function<void()> m_onClose{};
        std::wstring m_className{};
        std::wstring m_title{};
        int m_x{};
        int m_y{};
        int m_width{};
        int m_height{};
        /// @brief CreateWindowExW に渡すウィンドウスタイル（サブクラスで上書き可能）
        DWORD m_windowStyle{ WS_OVERLAPPEDWINDOW };
        HWND m_hwnd{};
        HICON m_hIcon{};

    private:
        static LRESULT CALLBACK staticWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    }; 

}