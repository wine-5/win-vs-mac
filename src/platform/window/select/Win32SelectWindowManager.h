#pragma once

#include "core/interface/ISelectWindowManager.h"
#include "core/data/FileExtensionType.h"
#include "game/utility/FileExtensionTypeResolver.h"
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
		  std::function<void(const std::string&)> onDifficultyChanged,
		  core::iface::IResourceManager& resourceManager,
		  core::iface::IScreen& screen,
		  bool showTutorial) noexcept;

	  virtual ~Win32SelectWindowManager() noexcept = default;

	  void createAllWindows() override;
	  void destroyAllWindows() override;
	  void pumpMessages() override;

	  void showWarningMessage(const std::string& message) noexcept override;

	  void setWindowsVisible(bool visible) noexcept override;

	private:
        // レイアウト定数
        static constexpr int TASKBAR_HEIGHT{ 48 };
        static constexpr int GAP_Y{ 8 };
        static constexpr int MARGIN_PERCENT{ 2 };
        static constexpr int COLUMN_COUNT{ 3 };
		// 左列上部をデスクトップアイコン用に空ける割合
		static constexpr int ICON_AREA_RATIO{ 11 };
		static constexpr int ICON_AREA_RATIO_BASE{ 20 };

		// RulesWindowのサイズ（クライアント領域に対する割合）。
		// 固定サイズだと、Debugの小さいウィンドウでは画面を覆い、
		// フルスクリーンでは読ませたい説明文が小さく浮くだけになる
		static constexpr int RULES_WINDOW_WIDTH_PERCENT{ 76 };
		static constexpr int RULES_WINDOW_HEIGHT_PERCENT{ 84 };

		// ウィンドウのアルファ値
        static constexpr BYTE WINDOW_ALPHA{ 250 };

        // ファイルスロット数
        static constexpr int FILE_SLOT_COUNT{ 3 };

		// 0.0〜1.0の確率を%表記へ直すための倍率（会心率の表示に使う）
		static constexpr float PERCENT_SCALE{ 100.0f };

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

		/**
		 * @brief パラメータウィンドウを最新の装備内容で更新する
		 *
		 * noexcept にはしない。ここで例外を握りつぶすと std::terminate になり、
		 * 原因がログにも残らないまま落ちるため、呼び出し元の catch まで通す
		 */
		void updateParameterWindow();

		/** @brief 装備済みスロット数を数える */
		[[nodiscard]] int countEquippedSlots() const noexcept;

		/**
		 * @brief 装備スロットが埋まっていない状態での開始確認ダイアログを出す
		 * @return 開始してよい場合true
		 */
		[[nodiscard]] bool confirmStartWithEmptySlots() noexcept;

		// DEBUG: セレクト画面を一時的に引っ込めるキー（裏のコンソールやダイアログを読むため）
		static constexpr int DEBUG_HIDE_KEY{ VK_F4 };

		/**
		 * @brief DEBUG: F4でセレクト画面の表示/非表示を切り替える（リリース時に削除）
		 *
		 * デスクトップは常時最前面のため、その裏のコンソールや例外ダイアログが読めない。
		 * 押した瞬間だけを拾って引っ込められるようにする
		 */
		void updateDebugOverlayToggle() noexcept;

		/**
		 * @brief 選択中の難易度を全ウィンドウへ配る
		 *
		 * HARDでは配色を警告色へ切り替えるため、デスクトップも含めた全画面が知る必要がある
		 * @param difficulty 難易度文字列（"NORMAL" | "HARD"）
		 */
		void broadcastDifficulty(const std::string& difficulty) noexcept;

		// 初回ガイドの段番号（file-tutorial.js の並びと対）
		static constexpr int TUTORIAL_STEP_BONUS{ 2 }; // 拡張子で能力が上がる（パラメータを強調）
		static constexpr int TUTORIAL_STEP_RULES{ 3 }; // ルール説明.txtへ誘導（デスクトップを強調）

		/**
		 * @brief 初回ガイドの段を各ウィンドウへ配り、強調表示を切り替えさせる
		 *
		 * ウィンドウは互いに直接やり取りできないため、ここが中継役になる
		 * @param step 段番号（1始まり・0はガイド終了）
		 */
		void broadcastTutorialStep(int step) noexcept;

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

		// DEBUG: F4での一時退避の状態（リリース時に削除）
		bool m_debugOverlayHidden{ false };
		bool m_debugHideKeyDown{ false };

		std::array<std::string, 3> m_slotPaths{};
		std::array<core::data::FileExtensionType, 3> m_slotExtTypes{
			core::data::FileExtensionType::Unknown,
			core::data::FileExtensionType::Unknown,
			core::data::FileExtensionType::Unknown
		};

		std::function<void()> m_onGameStart{};
        std::function<void(int, const std::string&)> m_onFileSlotChanged{};
		std::function<void(const std::string&)> m_onDifficultyChanged{};

		core::iface::IResourceManager& m_resourceManager;
        core::iface::IScreen& m_screen;

		// 初回だけ出す操作ガイドを表示するか（ファイル選択ウィンドウへ引き渡す）
		bool m_showTutorial{ false };
	};
} // namespace platform::window::select
