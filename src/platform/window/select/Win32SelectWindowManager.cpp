#include "Win32SelectWindowManager.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "core/constant/SelectWindowId.h"
#include "core/constant/JobType.h"
#include "thirdparty/nlohmann/json.hpp"

namespace platform::window::select
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
        int jobH{   availH / 2 };
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
        m_jobWindow->setOnJobSelect([this](core::constant::JobType jobType) noexcept {
            if (m_onJobSelect) m_onJobSelect(jobType);
            updateParameterWindowForJob(jobType);
        });
        m_jobWindow->setOnMinimize([this]() noexcept {
            m_jobWindow->hide();
            m_jobVisible = false;
            notifyWindowState("job", false);
        });
        m_jobWindow->setOnClose([this]() noexcept {
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
        m_fileSelectWindow->setOnFileSlotChanged([this](int slot, const std::string& path) noexcept {
            if (m_onFileSlotChanged) m_onFileSlotChanged(slot, path);
            if (slot >= 0 && slot < 3)
            {
                m_slotPaths[slot] = path;
                if (!path.empty())
                {
                    std::string ext{ path };
                    auto dotPos = ext.rfind('.');
                    if (dotPos != std::string::npos)
                    {
                        ext = ext.substr(dotPos);
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        m_slotExtTypes[slot] = game::utility::FileExtensionTypeResolver::toFileExtensionType(ext);
                    }
                    else
                    {
                        m_slotExtTypes[slot] = game::data::FileExtensionType::Unknown;
                    }
                }
                else
                {
                    m_slotExtTypes[slot] = game::data::FileExtensionType::Unknown;
                }
            }
            updateParameterWindowForJob(m_currentJobType);
        });
        m_fileSelectWindow->setOnMinimize([this]() noexcept {
            m_fileSelectWindow->hide();
            m_fileVisible = false;
            notifyWindowState("file", false);
        });
        m_fileSelectWindow->setOnClose([this]() noexcept {
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
        m_parameterWindow->setOnClose([this]() noexcept {
            m_paramVisible = false;
            notifyWindowState("param", false);
        });

        // DifficultyWindow (左列 下 / Job の下)
        m_difficultyWindow = std::make_unique<DifficultyWindow>(
            originX + marginX,
            winY + jobH + gapY,
            colWidth,
            fileH
        );
        if (!m_difficultyWindow->create(m_desktopWindow->getHwnd())) return;
        m_difficultyWindow->setOnMinimize([this]() noexcept {
            m_difficultyWindow->hide();
            m_diffVisible = false;
            notifyWindowState("diff", false);
        });
        m_difficultyWindow->setOnClose([this]() noexcept {
            m_diffVisible = false;
            notifyWindowState("diff", false);
        });

        m_jobWindow->show();
        m_fileSelectWindow->show();
        m_parameterWindow->show();
        m_difficultyWindow->show();
    }

    void Win32SelectWindowManager::destroyAllWindows()
    {
        if (m_jobWindow)          m_jobWindow->destroy();
        if (m_fileSelectWindow)   m_fileSelectWindow->destroy();
        if (m_parameterWindow)    m_parameterWindow->destroy();
        if (m_difficultyWindow)   m_difficultyWindow->destroy();
        if (m_desktopWindow)      m_desktopWindow->destroy();

        m_jobWindow.reset();
        m_fileSelectWindow.reset();
        m_parameterWindow.reset();
        m_difficultyWindow.reset();
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
        m_currentJobType = jobType;
        if (!m_parameterWindow) return;

        const core::data::JobInfo jobInfo{ m_resourceManager.getJobInfo(jobType) };

        float bonusHp{}, bonusAtk{}, bonusDef{}, bonusSpd{};
        int equippedCount{};
        for (int i = 0; i < 3; ++i)
        {
            if (!m_slotPaths[i].empty())
            {
                const auto bonus = game::utility::ExtensionBonusCalculator::calculate(m_slotExtTypes[i]);
                bonusHp  += bonus.hp;
                bonusAtk += bonus.atk;
                bonusDef += bonus.def;
                bonusSpd += bonus.spd;
                ++equippedCount;
            }
        }

        m_parameterWindow->refresh(
            jobInfo.m_hp,  jobInfo.m_atk,  jobInfo.m_def,  jobInfo.m_spd,
            bonusHp, bonusAtk, bonusDef, bonusSpd,
            jobInfo.m_name, jobInfo.m_skillName,
            equippedCount
        );
    }

    void Win32SelectWindowManager::handleDesktopMessage(const std::string& json) noexcept
    {
        try
        {
            auto j = nlohmann::json::parse(json);
            const std::string type{ j.value("type", "") };

            if (type == "startGame")
            {
                // ゲーム開始前に全サブウィンドウを非表示にしてからコールバックを呼ぶ
                if (m_desktopWindow && m_desktopWindow->getHwnd())
                    ShowWindow(m_desktopWindow->getHwnd(), SW_HIDE);
                if (m_jobWindow)        m_jobWindow->hide();
                if (m_fileSelectWindow) m_fileSelectWindow->hide();
                if (m_parameterWindow)  m_parameterWindow->hide();
                if (m_difficultyWindow) m_difficultyWindow->hide();
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
                else if (name == "diff" && m_difficultyWindow)
                {
                    m_diffVisible = !m_diffVisible;
                    m_diffVisible ? m_difficultyWindow->show() : m_difficultyWindow->hide();
                    notifyWindowState("diff", m_diffVisible);
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
