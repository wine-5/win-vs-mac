#pragma once

#include "core/interface/ISelectWindowManager.h"
#include "core/data/FileExtensionType.h"
#include "game/utility/FileExtensionTypeResolver.h"
#include "game/utility/ExtensionBonusCalculator.h"
#include "platform/window/WindowConstants.h"
#include "DesktopWindow.h"
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
} // namespace core::iface

namespace platform::window::select
{
    class Win32SelectWindowManager : public core::iface::ISelectWindowManager
    {
    public:
	  Win32SelectWindowManager(
		  std::function<void()> onGameStart,
		  std::function<void(int, const std::string&)> onFileSlotChanged,
		  core::iface::IResourceManager& resourceManager,
		  core::iface::IScreen& screen) noexcept;

	  virtual ~Win32SelectWindowManager() noexcept = default;

	  void createAllWindows() override;
	  void destroyAllWindows() override;
	  void pumpMessages() override;

	  void showWarningMessage(const std::string& message) noexcept override;

    private:
        // レイアウト定数
        static constexpr int TASKBAR_HEIGHT{ 48 };
        static constexpr int GAP_Y{ 8 };
        static constexpr int MARGIN_PERCENT{ 2 };
        static constexpr int COLUMN_COUNT{ 3 };
		// 左列上部をデスクトップアイコン用に空ける割合
		static constexpr int ICON_AREA_RATIO{ 11 };
		static constexpr int ICON_AREA_RATIO_BASE{ 20 };

		// RulesWindowのサイズ
        static constexpr int RULES_WINDOW_WIDTH{ 920 };
        static constexpr int RULES_WINDOW_HEIGHT{ 660 };

        // ウィンドウのアルファ値
        static constexpr BYTE WINDOW_ALPHA{ 250 };

        // ファイルスロット数
        static constexpr int FILE_SLOT_COUNT{ 3 };

        // ウィンドウ名
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

		/** @brief パラメータウィンドウを最新の装備内容で更新する */
		void updateParameterWindow() noexcept;

		/** @brief 装備済みスロット数を数える */
		[[nodiscard]] int countEquippedSlots() const noexcept;

		/**
		 * @brief 装備スロットが埋まっていない状態での開始確認ダイアログを出す
		 * @return 開始してよい場合true
		 */
		[[nodiscard]] bool confirmStartWithEmptySlots() noexcept;

		void handleDesktopMessage(const std::string& json) noexcept;
        void notifyWindowState(const std::string& name, bool visible) noexcept;

        std::unique_ptr<DesktopWindow>    m_desktopWindow{};
        std::unique_ptr<FileSelectWindow> m_fileSelectWindow{};
        std::unique_ptr<ParameterWindow>  m_parameterWindow{};
        std::unique_ptr<DifficultyWindow> m_difficultyWindow{};
        std::unique_ptr<RulesWindow>      m_rulesWindow{};

        bool m_fileVisible{true};
        bool m_paramVisible{true};
        bool m_diffVisible{true};
        bool m_rulesVisible{false};

        std::array<std::string, 3> m_slotPaths{};
		std::array<core::data::FileExtensionType, 3> m_slotExtTypes{
			core::data::FileExtensionType::Unknown,
			core::data::FileExtensionType::Unknown,
			core::data::FileExtensionType::Unknown
		};

		std::function<void()> m_onGameStart{};
        std::function<void(int, const std::string&)> m_onFileSlotChanged{};

        core::iface::IResourceManager& m_resourceManager;
        core::iface::IScreen& m_screen;
    };
} // namespace platform::window::select
