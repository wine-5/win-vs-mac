#pragma once

#include <windows.h>
#include <string>
#include "WindowBase.h"
#include "platform/webview/WebView2Host.h"

namespace core::data
{
    struct JobInfo;
}

namespace platform::window
{
    /**
     * @class ParameterWindow
     * @brief ステータス表示ウィンドウ
     */
    class ParameterWindow : public WindowBase
    {
    public:
        /**
         * @brief コンストラクタ
         * @param x ウィンドウの左上角 X 座標
         * @param y ウィンドウの左上角 Y 座標
         * @param width ウィンドウの幅
         * @param height ウィンドウの高さ
         */
        ParameterWindow(int x, int y, int width, int height) noexcept;

        /// @brief デストラクタ
        virtual ~ParameterWindow() noexcept = default;

        /**
         * @brief ステータス情報を更新
         * @param jobInfo 職業情報
         */
        void refresh(const core::data::JobInfo& jobInfo) noexcept;

    protected:
        void onCreateControls(HWND hwnd) override;
        LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;

    private:
        static constexpr const wchar_t* ICON_PATH = L"assets/images/ui/icons/param.ico";

        platform::webview::WebView2Host m_webView{};
    };
}
