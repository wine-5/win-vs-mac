#pragma once

#include <windows.h>
#include "WindowBase.h"
#include <functional>

namespace core::constant
{
    enum class JobType;
}

namespace platform::window
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
        HWND m_job1Button{};
        HWND m_job2Button{};
        HWND m_job3Button{};
        core::constant::JobType m_selectedJob{};
        std::function<void(core::constant::JobType)> m_onJobSelect{};

        static constexpr int IDC_JOB1_BUTTON = 2001;
        static constexpr int IDC_JOB2_BUTTON = 2002;
        static constexpr int IDC_JOB3_BUTTON = 2003;
    };
}