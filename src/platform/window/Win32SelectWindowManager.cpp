#include "Win32SelectWindowManager.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "core/constant/SelectWindowId.h"
#include "core/constant/JobType.h"

namespace platform::window
{
    Win32SelectWindowManager::Win32SelectWindowManager(
        std::function<void()> onGameStart,
        std::function<void(core::constant::JobType)> onJobSelect,
        std::function<void(int, const std::string&)> onFileSlotChanged,
        core::iface::IResourceManager& resourceManager,
        core::iface::IScreen& screen
    ) noexcept
        : m_onGameStart(onGameStart),
        m_onJobSelect(onJobSelect),
        m_onFileSlotChanged(onFileSlotChanged),
        m_resourceManager(resourceManager),
        m_screen(screen)
    {
    }

    void Win32SelectWindowManager::createAllWindows()
    {
        HWND dxlibHwnd = static_cast<HWND>(m_screen.getNativeWindowHandle());

        RECT clientRect{};
        GetClientRect(dxlibHwnd, &clientRect);
        int screenWidth{ clientRect.right };
        int screenHeight{ clientRect.bottom };

        POINT origin{ 0, 0 };
        ClientToScreen(dxlibHwnd, &origin);
        int originX{ origin.x };
        int originY{ origin.y };

        int marginX{ screenWidth * 1 / 100 };
        int marginY{ screenHeight * 1 / 100 };
        int leftPanelWidth{ screenWidth * 19 / 100 };
        int centerWidth{ screenWidth * 43 / 100 };
        int rightPanelWidth{ screenWidth * 34 / 100 };

        int usableHeight{ screenHeight - marginY * 2 };
        int jobWindowHeight{ usableHeight / 2 - marginY / 2 };
        int fileSelectWindowHeight{ usableHeight / 2 - marginY / 2 };

        m_selectWindow = std::make_unique<SelectWindow>(
            originX + marginX + leftPanelWidth + marginX,
            originY + marginY,
            centerWidth,
            usableHeight
        );
        if (!m_selectWindow->create()) return;
        m_selectWindow->setOnGameStart(m_onGameStart);

        m_jobWindow = std::make_unique<JobWindow>(
            originX + marginX,
            originY + marginY,
            leftPanelWidth,
            jobWindowHeight
        );
        if (!m_jobWindow->create()) return;
        m_jobWindow->setOnJobSelect(m_onJobSelect);

        m_fileSelectWindow = std::make_unique<FileSelectWindow>(
            originX + marginX,
            originY + marginY + jobWindowHeight + marginY,
            leftPanelWidth,
            fileSelectWindowHeight
        );
        if (!m_fileSelectWindow->create()) return;
        m_fileSelectWindow->setOnFileSlotChanged(m_onFileSlotChanged);

        m_parameterWindow = std::make_unique<ParameterWindow>(
            originX + marginX + leftPanelWidth + marginX + centerWidth + marginX,
            originY + marginY,
            rightPanelWidth,
            usableHeight
        );
        if (!m_parameterWindow->create()) return;

        m_selectWindow->show();
        m_jobWindow->show();
        m_fileSelectWindow->show();
        m_parameterWindow->show();
    }

    void Win32SelectWindowManager::destroyAllWindows()
    {
        if (m_selectWindow)
            m_selectWindow->destroy();
        if (m_jobWindow)
            m_jobWindow->destroy();
        if (m_fileSelectWindow)
            m_fileSelectWindow->destroy();
        if (m_parameterWindow)
            m_parameterWindow->destroy();

        m_selectWindow.reset();
        m_jobWindow.reset();
        m_fileSelectWindow.reset();
        m_parameterWindow.reset();
    }

    void Win32SelectWindowManager::pumpMessages()
    {
        MSG msg{};
        while (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                continue;

            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
    }

    void Win32SelectWindowManager::bringToFront(core::constant::SelectWindowId id)
    {
        switch (id)
        {
        case core::constant::SelectWindowId::Stage:
            if (m_selectWindow)
                m_selectWindow->bringToFront();
            break;

        case core::constant::SelectWindowId::Job:
            if (m_jobWindow)
                m_jobWindow->bringToFront();
            break;

        case core::constant::SelectWindowId::FileSelect:
            if (m_fileSelectWindow)
                m_fileSelectWindow->bringToFront();
            break;

        case core::constant::SelectWindowId::Parameter:
            if (m_parameterWindow)
                m_parameterWindow->bringToFront();
            break;
        }
    }

    void Win32SelectWindowManager::updateParameterWindowForJob(core::constant::JobType jobType) noexcept
    {
        if (!m_parameterWindow) return;
        core::data::JobInfo jobInfo{ m_resourceManager.getJobInfo(jobType) };
        m_parameterWindow->refresh(jobInfo);
    }
}
