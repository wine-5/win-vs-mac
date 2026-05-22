#pragma once

#include <windows.h>
#include <optional>
#include <string>
#include "platform/window/WindowConstants.h"
#include "platform/window/WindowBase.h"
#include "platform/webview/WebView2Host.h"
#include "infrastructure/repository/JobRepository.h"
#include <functional>

namespace core::constant
{
    enum class JobType;
}

namespace platform::window::select
{
    /**
     * @class JobWindow
     * @brief 職業選択ウィンドウ
     */
    class JobWindow : public WindowBase
    {
    public:
        /**
         * @brief コンストラクタ
         * @param x ウィンドウの左上角 X 座標
         * @param y ウィンドウの左上角 Y 座標
         * @param width ウィンドウの幅
         * @param height ウィンドウの高さ
         */
        JobWindow(int x, int y, int width, int height) noexcept;

        /// @brief デストラクタ
        virtual ~JobWindow() noexcept = default;

        /**
         * @brief 職業選択時のコールバック設定
         * @param callback 選択された職業を受け取るコールバック関数
         */
        void setOnJobSelect(std::function<void(core::constant::JobType)> callback) noexcept;

        /**
         * @brief 選択された職業を取得
         * @return 職業タイプ
         */
        core::constant::JobType getSelectedJob() const noexcept;

    protected:
        void onCreateControls(HWND hwnd) override;
        LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;

    private:
        // ウィンドウ定数
        static constexpr const wchar_t* ICON_PATH{ L"assets/images/ui/icons/job.ico" };
        static constexpr const wchar_t* WINDOW_CLASS_NAME{ L"JobWindowClass" };
        static constexpr const wchar_t* WINDOW_TITLE{ L"職業選択" };
        static constexpr const wchar_t* JOB_HTML_URL{ L"https://game.web/select/job/job.html" };

        // ジョブ名
        static constexpr const char* JOB_NAME_WARRIOR{ "Warrior" };
        static constexpr const char* JOB_NAME_MAGE{ "Mage" };
        static constexpr const char* JOB_NAME_NINJA{ "Ninja" };

        platform::webview::WebView2Host m_webView{};
        core::constant::JobType m_selectedJob{};
        std::function<void(core::constant::JobType)> m_onJobSelect{};
        std::optional<infrastructure::JobRepository> m_jobRepository{};

        void handleMessage(const std::string& json) noexcept;
        void sendJobStats() noexcept;
    };
}