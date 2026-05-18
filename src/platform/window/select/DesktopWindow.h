#pragma once

#include <windows.h>
#include <string>
#include <functional>
#include "platform/webview/WebView2Host.h"

namespace platform::window::select
{
    /**
     * @brief デスクトップ背景ウィンドウ
     * @details ボーダーレス WS_POPUP ウィンドウ。DxLib のクライアント領域全体を
     *          覆い、WebView2 で desktop.html を表示する。
     */
    class DesktopWindow
    {
    public:
        DesktopWindow() noexcept = default;
        ~DesktopWindow() noexcept;
        DesktopWindow(const DesktopWindow&) = delete;
        DesktopWindow& operator=(const DesktopWindow&) = delete;

        /**
         * @brief ウィンドウを作成し WebView2 を初期化する
         * @param x スクリーン座標 X
         * @param y スクリーン座標 Y
         * @param width 幅
         * @param height 高さ
         * @return 成功時 true
         */
        bool create(int x, int y, int width, int height) noexcept;

        /**
         * @brief ウィンドウを破棄する
         */
        void destroy() noexcept;

        /**
         * @brief ウィンドウを表示する
         */
        void show() noexcept;

        /**
         * @brief JS へ UTF-8 JSON を送信する
         * @param utf8Json 送信する JSON 文字列
         */
        void postMessage(const std::string& utf8Json) noexcept;

        /**
         * @brief JS からのメッセージコールバックを設定する
         * @param callback JSON 文字列（UTF-8）を受け取るコールバック
         */
        void setOnMessage(std::function<void(const std::string&)> callback) noexcept;

        /**
         * @brief ウィンドウハンドルを取得する
         * @return HWND
         */
        [[nodiscard]] HWND getHwnd() const noexcept;

        /**
         * @brief 作成済みか
         * @return 作成済みなら true
         */
        [[nodiscard]] bool isCreated() const noexcept;

    private:
        static LRESULT CALLBACK staticWindowProc(
            HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
        LRESULT windowProc(
            HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

        HWND m_hwnd{};
        platform::webview::WebView2Host m_webView{};

        static constexpr wchar_t CLASS_NAME[] = L"DesktopWindowClass";
    };
}
