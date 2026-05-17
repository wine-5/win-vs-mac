#include "Win32SelectWindowManager.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "core/constant/SelectWindowId.h"
#include "core/constant/JobType.h"
#include "thirdparty/nlohmann/json.hpp"

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

        // --- DesktopWindow: DxLib クライアント領域全体を覆う ---
        m_desktopWindow = std::make_unique<DesktopWindow>();
        m_desktopWindow->setOnMessage([this](const std::string& json) noexcept {
            handleDesktopMessage(json);
        });
        if (!m_desktopWindow->create(originX, originY, screenWidth, screenHeight)) return;
        m_desktopWindow->show();

        // --- レイアウト定数 ---
        // 左列: Job(上) + File(下)  /  中央列: HTML難易度パネル  /  右列: Parameter
        constexpr int taskbarH{ 48 };
        constexpr int gapY{ 8 };
        int marginX{ screenWidth  * 2 / 100 };
        int marginY{ screenHeight * 2 / 100 };
        int colWidth{ (screenWidth  - marginX * 4) / 3 };
        int availH{    screenHeight - taskbarH - marginY * 2 };
        int jobH{   availH * 3 / 5 };
        int fileH{  availH - jobH - gapY };

        int winY{ originY + marginY };

        // Job ウィンドウ（左列 上）
        m_jobWindow = std::make_unique<JobWindow>(
            originX + marginX,
            winY,
            colWidth,
            jobH
        );
        if (!m_jobWindow->create(m_desktopWindow->getHwnd())) return;
        m_jobWindow->setOnJobSelect(m_onJobSelect);
        m_jobWindow->setOnMinimize([this]() noexcept {
            m_jobWindow->hide();
            m_jobVisible = false;
            notifyWindowState("job", false);
        });

        // FileSelect ウィンドウ（中央列 全高）
        m_fileSelectWindow = std::make_unique<FileSelectWindow>(
            originX + marginX * 2 + colWidth,
            winY,
            colWidth,
            availH
        );
        if (!m_fileSelectWindow->create(m_desktopWindow->getHwnd())) return;
        m_fileSelectWindow->setOnFileSlotChanged(m_onFileSlotChanged);
        m_fileSelectWindow->setOnMinimize([this]() noexcept {
            m_fileSelectWindow->hide();
            m_fileVisible = false;
            notifyWindowState("file", false);
        });

        // Parameter ウィンドウ（右列 全高）
        m_parameterWindow = std::make_unique<ParameterWindow>(
            originX + marginX * 3 + colWidth * 2,
            winY,
            colWidth,
            availH
        );
        if (!m_parameterWindow->create(m_desktopWindow->getHwnd())) return;
        m_parameterWindow->setOnMinimize([this]() noexcept {
            m_parameterWindow->hide();
            m_paramVisible = false;
            notifyWindowState("param", false);
        });

        m_jobWindow->show();
        m_fileSelectWindow->show();
        m_parameterWindow->show();
    }

    void Win32SelectWindowManager::destroyAllWindows()
    {
        if (m_jobWindow)        m_jobWindow->destroy();
        if (m_fileSelectWindow) m_fileSelectWindow->destroy();
        if (m_parameterWindow)  m_parameterWindow->destroy();
        if (m_desktopWindow)    m_desktopWindow->destroy();

        m_jobWindow.reset();
        m_fileSelectWindow.reset();
        m_parameterWindow.reset();
        m_desktopWindow.reset();
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
        case core::constant::SelectWindowId::Job:
            if (m_jobWindow) m_jobWindow->bringToFront();
            break;
        case core::constant::SelectWindowId::FileSelect:
            if (m_fileSelectWindow) m_fileSelectWindow->bringToFront();
            break;
        case core::constant::SelectWindowId::Parameter:
            if (m_parameterWindow) m_parameterWindow->bringToFront();
            break;
        default:
            break;
        }
    }

    void Win32SelectWindowManager::updateParameterWindowForJob(
        core::constant::JobType jobType) noexcept
    {
        if (!m_parameterWindow) return;
        core::data::JobInfo jobInfo{ m_resourceManager.getJobInfo(jobType) };
        m_parameterWindow->refresh(jobInfo);
    }

    void Win32SelectWindowManager::handleDesktopMessage(const std::string& json) noexcept
    {
        try
        {
            auto j = nlohmann::json::parse(json);
            const std::string type{ j.value("type", "") };

            if (type == "startGame")
            {
                if (m_onGameStart) m_onGameStart();
            }
            else if (type == "toggleWindow")
            {
                const std::string name{ j.value("window", "") };
                if (name == "job" && m_jobWindow)
                {
                    m_jobVisible = !m_jobVisible;
                    m_jobVisible ? m_jobWindow->show() : m_jobWindow->hide();
                    notifyWindowState("job", m_jobVisible);
                }
                else if (name == "file" && m_fileSelectWindow)
                {
                    m_fileVisible = !m_fileVisible;
                    m_fileVisible ? m_fileSelectWindow->show() : m_fileSelectWindow->hide();
                    notifyWindowState("file", m_fileVisible);
                }
                else if (name == "param" && m_parameterWindow)
                {
                    m_paramVisible = !m_paramVisible;
                    m_paramVisible ? m_parameterWindow->show() : m_parameterWindow->hide();
                    notifyWindowState("param", m_paramVisible);
                }
            }
        }
        catch (...) {}
    }

    void Win32SelectWindowManager::notifyWindowState(
        const std::string& name, bool visible) noexcept
    {
        if (!m_desktopWindow) return;
        try
        {
            nlohmann::json j;
            j["type"]    = "windowStateChanged";
            j["window"]  = name;
            j["visible"] = visible;
            m_desktopWindow->postMessage(j.dump());
        }
        catch (...) {}
    }
}
