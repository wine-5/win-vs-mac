#pragma once

#include "core/interface/ISelectWindowManager.h"
#include "core/constant/JobType.h"
#include "core/data/JobInfo.h"
#include "game/data/FileExtensionType.h"
#include "game/utility/FileExtensionTypeResolver.h"
#include "game/utility/ExtensionBonusCalculator.h"
#include "DesktopWindow.h"
#include "JobWindow.h"
#include "FileSelectWindow.h"
#include "ParameterWindow.h"
#include "DifficultyWindow.h"
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

    private:
        void handleDesktopMessage(const std::string& json) noexcept;
        void notifyWindowState(const std::string& name, bool visible) noexcept;

        std::unique_ptr<DesktopWindow>      m_desktopWindow{};
        std::unique_ptr<JobWindow>          m_jobWindow{};
        std::unique_ptr<FileSelectWindow>   m_fileSelectWindow{};
        std::unique_ptr<ParameterWindow>    m_parameterWindow{};
        std::unique_ptr<DifficultyWindow>   m_difficultyWindow{};

        bool m_jobVisible{true};
        bool m_fileVisible{true};
        bool m_paramVisible{true};
        bool m_diffVisible{true};

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
