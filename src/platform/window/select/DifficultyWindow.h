#pragma once

#include "platform/window/WindowConstants.h"
#include "platform/window/WindowBase.h"
#include "platform/webview/WebView2Host.h"
#include <string>
#include <functional>

namespace platform::window::select
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
        // ウィンドウ定数
        static constexpr const wchar_t* ICON_PATH{ L"assets/images/ui/icons/diff.ico" };
        static constexpr const wchar_t* WINDOW_CLASS_NAME{ L"DifficultyWindowClass" };
        static constexpr const wchar_t* WINDOW_TITLE{ L"難易度設定" };
        static constexpr const wchar_t* DIFFICULTY_HTML_URL{ L"https://game.web/select/difficulty/difficulty.html" };

        // JSONメッセージタイプ
        static constexpr const char* MESSAGE_TYPE_DIFFICULTY_CHANGED{ "difficultyChanged" };
        static constexpr const char* MESSAGE_TYPE_CONFIRM_HARD{ "confirmHard" };
        static constexpr const char* MESSAGE_TYPE_HARD_CONFIRMED{ "hardConfirmed" };

        // 難易度値
        static constexpr const char* DIFFICULTY_EASY{ "EASY" };
        static constexpr const char* DIFFICULTY_NORMAL{ "NORMAL" };
        static constexpr const char* DIFFICULTY_HARD{ "HARD" };

        // メッセージボックステキスト
        static constexpr const wchar_t* HARD_WARNING_MESSAGE{
            L"HARD モードを選択しようとしています。\n"
            L"敵の攻撃力・防御力が大幅に上昇し、\n"
            L"攻略が非常に困難になります。\n\n"
            L"本当に続行しますか？"
        };
        static constexpr const wchar_t* HARD_WARNING_TITLE{ L"警告 - Win vs Mac.exe" };

        platform::webview::WebView2Host m_webView{};
        std::string m_selectedDifficulty{ DIFFICULTY_NORMAL };

        void handleMessage(const std::string& json) noexcept;
    };
} // namespace platform::window::select
