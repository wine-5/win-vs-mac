#pragma once

#include "platform/window/WindowBase.h"
#include "platform/webview/WebView2Host.h"

namespace platform::window::select
{
    /**
     * @class RulesWindow
     * @brief ルール説明ウィンドウ（メモ帳風）
     */
    class RulesWindow : public WindowBase
    {
    public:
        /**
         * @brief コンストラクタ
         * @param x ウィンドウの左上角 X 座標
         * @param y ウィンドウの左上角 Y 座標
         * @param width ウィンドウの幅
         * @param height ウィンドウの高さ
         */
        RulesWindow(int x, int y, int width, int height) noexcept;

        /// @brief デストラクタ
        virtual ~RulesWindow() noexcept = default;

    protected:
        void onCreateControls(HWND hwnd) override;
        LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;

    private:
        static constexpr const wchar_t* ICON_PATH = L"assets/images/ui/icons/Rules.ico";

        platform::webview::WebView2Host m_webView{};
    };
}
