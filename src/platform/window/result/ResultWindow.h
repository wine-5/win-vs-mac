#pragma once

#include "platform/window/WindowBase.h"
#include "platform/webview/WebView2Host.h"
#include "core/interface/IResultWindowManager.h"
#include "core/interface/IScreen.h"

#include <functional>
#include <optional>

namespace platform::window::result
{
    /**
     * @class ResultWindow
     * @brief リザルト画面を表示する WebView2 ウィンドウ
     */
    class ResultWindow : public WindowBase, public core::iface::IResultWindowManager
    {
    public:
        /**
         * @brief コンストラクタ
         * @param screen 画面情報インターフェース（位置計算・オーナーHWND取得に使用）
         * @param onRetry  「もう一度」ボタン押下時のコールバック
         * @param onTitle  「タイトルへ」ボタン押下時のコールバック
         */
        ResultWindow(
            core::iface::IScreen& screen,
            std::function<void()> onRetry,
            std::function<void()> onTitle) noexcept;

        virtual ~ResultWindow() noexcept = default;

        /**
         * @brief ウィンドウを表示し、結果データを WebView に送信する
         * @param data リザルトデータ
         */
        void show(const core::data::ResultData& data) noexcept override;

        /**
         * @brief メッセージポンプ（毎フレーム呼び出し）
         */
        void pumpMessages() noexcept override;

    protected:
        void onCreateControls(HWND hwnd) override;
        LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;

    private:

        core::iface::IScreen& m_screen;
        platform::webview::WebView2Host m_webView{};

        std::function<void()> m_onRetry{};
        std::function<void()> m_onTitle{};

        std::optional<core::data::ResultData> m_pendingData{};

        void handleMessage(const std::string& json) noexcept;
        void sendResultData(const core::data::ResultData& data) noexcept;
    };
}
