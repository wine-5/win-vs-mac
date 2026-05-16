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
    class JobWindow : public WindowBase
    {
    public:
        JobWindow(int x, int y, int width, int height) noexcept;
        virtual ~JobWindow() noexcept = default;

        void setOnJobSelect(std::function<void(core::constant::JobType)> callback) noexcept;
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