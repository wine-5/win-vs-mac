#pragma once

#include "core/interface/ISelectWindowManager.h"
#include "core/constant/JobType.h"
#include "core/data/JobInfo.h"
#include "game/data/FileExtensionType.h"
#include "game/utility/FileExtensionTypeResolver.h"
#include "game/utility/ExtensionBonusCalculator.h"
#include "platform/window/WindowConstants.h"
#include "DesktopWindow.h"
#include "JobWindow.h"
#include "FileSelectWindow.h"
#include "ParameterWindow.h"
#include "DifficultyWindow.h"
#include "RulesWindow.h"
#include <memory>
#include <functional>
#include <string>
#include <array>
#include <algorithm>

namespace core::iface
{
    class IResourceManager;
    class IScreen;
}

namespace platform::window::select
{
    class Win32SelectWindowManager : public core::iface::ISelectWindowManager
    {
    public:
        Win32SelectWindowManager(
            std::function<void()> onGameStart,
            std::function<void(core::constant::JobType)> onJobSelect,
            std::function<void(int, const std::string&)> onFileSlotChanged,
            core::iface::IResourceManager& resourceManager,
            core::iface::IScreen& screen
        ) noexcept;

        virtual ~Win32SelectWindowManager() noexcept = default;

        void createAllWindows() override;
        void destroyAllWindows() override;
        void pumpMessages() override;
        void bringToFront(core::constant::SelectWindowId id) override;

        void updateParameterWindowForJob(core::constant::JobType jobType) noexcept;
        bool isJobSelected() const noexcept override { return m_jobSelected; }
        void showWarningMessage(const std::string& message) noexcept override;

    private:
        // レイアウト定数
        static constexpr int TASKBAR_HEIGHT{ 48 };
        static constexpr int GAP_Y{ 8 };
        static constexpr int MARGIN_PERCENT{ 2 };
        static constexpr int COLUMN_COUNT{ 3 };
        static constexpr int JOB_HEIGHT_RATIO{ 11 };
        static constexpr int JOB_HEIGHT_RATIO_BASE{ 20 };

        // RulesWindowのサイズ
        static constexpr int RULES_WINDOW_WIDTH{ 920 };
        static constexpr int RULES_WINDOW_HEIGHT{ 660 };

        // ウィンドウのアルファ値
        static constexpr BYTE WINDOW_ALPHA{ 250 };

        // ファイルスロット数
        static constexpr int FILE_SLOT_COUNT{ 3 };

        // ウィンドウ名
        static constexpr const char* WINDOW_NAME_JOB{ "job" };
        static constexpr const char* WINDOW_NAME_FILE{ "file" };
        static constexpr const char* WINDOW_NAME_PARAM{ "param" };
        static constexpr const char* WINDOW_NAME_DIFF{ "diff" };
        static constexpr const char* WINDOW_NAME_RULES{ "rules" };

        // アプリケーション名とパス
        // アプリパス（複雑で再利用可能）
        static constexpr const wchar_t* APP_CMD_PATH{ L"cmd.exe" };
        static constexpr const wchar_t* APP_TASKMGR_PATH{ L"taskmgr.exe" };
        static constexpr const wchar_t* APP_RECYCLEBIN_PATH{ L"shell:RecycleBinFolder" };
        static constexpr const wchar_t* APP_NOTEPAD_PATH{ L"notepad.exe" };

        void handleDesktopMessage(const std::string& json) noexcept;
        void notifyWindowState(const std::string& name, bool visible) noexcept;

        std::unique_ptr<DesktopWindow>    m_desktopWindow{};
        std::unique_ptr<JobWindow>        m_jobWindow{};
        std::unique_ptr<FileSelectWindow> m_fileSelectWindow{};
        std::unique_ptr<ParameterWindow>  m_parameterWindow{};
        std::unique_ptr<DifficultyWindow> m_difficultyWindow{};
        std::unique_ptr<RulesWindow>      m_rulesWindow{};

        bool m_jobVisible{true};
        bool m_fileVisible{true};
        bool m_paramVisible{true};
        bool m_diffVisible{true};
        bool m_rulesVisible{false};

        bool m_jobSelected{false};
        core::constant::JobType m_currentJobType{core::constant::JobType::Warrior};
        std::array<std::string, 3> m_slotPaths{};
        std::array<game::data::FileExtensionType, 3> m_slotExtTypes{
            game::data::FileExtensionType::Unknown,
            game::data::FileExtensionType::Unknown,
            game::data::FileExtensionType::Unknown
        };

        std::function<void()> m_onGameStart{};
        std::function<void(core::constant::JobType)> m_onJobSelect{};
        std::function<void(int, const std::string&)> m_onFileSlotChanged{};

        core::iface::IResourceManager& m_resourceManager;
        core::iface::IScreen& m_screen;
    };
}
