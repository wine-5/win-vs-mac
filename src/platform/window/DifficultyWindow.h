#pragma once

#include "WindowBase.h"
#include "platform/webview/WebView2Host.h"
#include <string>
#include <functional>

namespace platform::window
{
    /**
     * @class DifficultyWindow
     * @brief 難易度選択ウィンドウ
     */
    class DifficultyWindow : public WindowBase
    {
    public:
        /**
         * @brief コンストラクタ
         * @param x ウィンドウの左上角 X 座標
         * @param y ウィンドウの左上角 Y 座標
         * @param width ウィンドウの幅
         * @param height ウィンドウの高さ
         */
        DifficultyWindow(int x, int y, int width, int height) noexcept;

        /// @brief デストラクタ
        virtual ~DifficultyWindow() noexcept = default;

        /**
         * @brief 現在選択されている難易度を取得する
         * @return 難易度文字列 ("EASY" | "NORMAL" | "HARD")
         */
        [[nodiscard]] std::string getSelectedDifficulty() const noexcept;

    protected:
        void onCreateControls(HWND hwnd) override;
        LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;

    private:
        static constexpr const wchar_t* ICON_PATH = L"assets/images/ui/icons/diff.ico";

        platform::webview::WebView2Host m_webView{};
        std::string m_selectedDifficulty{"NORMAL"};

        void handleMessage(const std::string& json) noexcept;
    };
}
