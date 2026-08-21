#pragma once

#include "core/interface/ISelectWindowManager.h"
#include "core/data/FileExtensionType.h"
#include "core/data/GameSettings.h"
#include "game/utility/FileExtensionTypeResolver.h"
#include "platform/window/WindowConstants.h"
#include "DesktopWindow.h"
#include "FileSelectWindow.h"
#include "ParameterWindow.h"
#include "DifficultyWindow.h"
#include "QuickSettingsWindow.h"
#include "RulesWindow.h"
#include "SettingsWindow.h"
#include <memory>
#include <functional>
#include <string>
#include <array>
#include <vector>
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
		  std::function<void()> onBackToTitle,
		  std::function<void()> onQuitGame,
		  std::function<void(int, const std::string&)> onFileSlotChanged,
		  std::function<void(const std::string&)> onDifficultyChanged,
		  std::function<core::data::GameSettings()> getSettings,
		  std::function<void(const core::data::GameSettings&)> onSettingsChanged,
		  core::iface::IResourceManager& resourceManager,
		  core::iface::IScreen& screen,
		  bool showTutorial) noexcept;

	  virtual ~Win32SelectWindowManager() noexcept = default;

	  void createAllWindows() override;
	  void destroyAllWindows() override;
	  void pumpMessages() override;

	  void setModalInputPump(std::function<void(float)> pump) noexcept override;

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

		// 設定ウィンドウのサイズ。タイトル・ポーズで出るDxLib側の設定画面と同じ割合にして、
		// どこから開いても同じ大きさのものが出るようにする
		static constexpr int SETTINGS_WINDOW_WIDTH_PERCENT{ 66 };
		static constexpr int SETTINGS_WINDOW_HEIGHT_PERCENT{ 82 };

		// クイック設定の大きさ（画面幅に対する割合と、内容が収まる高さ）。
		// Windowsのものと同じく、タスクバーの右上へ小さく出す
		static constexpr int QUICK_WINDOW_WIDTH_PERCENT{ 22 };
		static constexpr int QUICK_WINDOW_MIN_WIDTH{ 280 };
		static constexpr int QUICK_WINDOW_HEIGHT{ 104 };
		static constexpr int QUICK_WINDOW_MARGIN{ 12 };

		// ウィンドウのアルファ値
        static constexpr BYTE WINDOW_ALPHA{ 250 };

        // ファイルスロット数
        static constexpr int FILE_SLOT_COUNT{ 3 };

		// 0.0〜1.0の確率を%表記へ直すための倍率（クリティカル率の表示に使う）
		static constexpr float PERCENT_SCALE{ 100.0f };

		// ウィンドウ名
        static constexpr const char* WINDOW_NAME_FILE{ "file" };
        static constexpr const char* WINDOW_NAME_PARAM{ "param" };
        static constexpr const char* WINDOW_NAME_DIFF{ "diff" };
        static constexpr const char* WINDOW_NAME_RULES{ "rules" };
		static constexpr const char* WINDOW_NAME_SETTINGS{ "settings" };
		static constexpr const char* WINDOW_NAME_QUICK{ "quick" };

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

		/** @brief 確認ダイアログ中にカーソルを動かす間隔（ミリ秒） */
		static constexpr UINT MODAL_PUMP_INTERVAL_MS{ 16 };

		/** @brief 上の間隔を秒で表したもの。止まっている間は実時間を測れないので固定値を渡す */
		static constexpr float MODAL_PUMP_DELTA{ 0.016f };

		/**
		 * @brief はい／いいえの確認ダイアログを出す
		 *
		 * 出している間もパッドでカーソルを動かせるよう、タイマーを仕掛けてから出す
		 * @param text 本文
		 * @param caption 見出し
		 * @return OKを選んだならtrue
		 */
		[[nodiscard]] bool showConfirmDialog(const wchar_t* text, const wchar_t* caption) noexcept;

		/**
		 * @brief 確認ダイアログを出している間だけ、入力を回すタイマーを仕掛ける
		 *
		 * ウィンドウを指定せずにタイマーを張ると、WM_TIMER は DispatchMessage から
		 * 直接この手続きへ渡される。ダイアログのモーダルループも DispatchMessage を
		 * 呼ぶので、ゲームのループが止まっている間も動く
		 */
		void beginModalInputPump() noexcept;

		/** @brief 確認ダイアログを閉じたあとにタイマーを止める */
		void endModalInputPump() noexcept;

		/**
		 * @brief タイマーから呼ばれ、入力を1回ぶん回す
		 * @param hwnd 使わない（ウィンドウ無しのタイマーのため）
		 * @param msg 使わない
		 * @param timerId 使わない
		 * @param elapsed 使わない
		 */
		static void CALLBACK modalInputPumpProc(
		    HWND hwnd, UINT msg, UINT_PTR timerId, DWORD elapsed) noexcept;

		/**
		 * @brief 出撃前の確認ダイアログを出す
		 *
		 * デスクトップ側のHTMLではなくWin32のダイアログで出す。セレクト画面の各ウィンドウは
		 * それぞれ別のHWNDなので、HTMLで出した確認はファイル選択などの背面に回ってしまう。
		 *
		 * @return 開始してよい場合true
		 */
		[[nodiscard]] bool confirmStart() noexcept;

		/**
		 * @brief タイトルへ戻る前の確認ダイアログを出す
		 *
		 * 戻ると装備も難易度も選び直しになるため、確認を挟む
		 * @return 戻ってよい場合true
		 */
		[[nodiscard]] bool confirmBackToTitle() noexcept;

		/**
		 * @brief アプリを終了する前の確認ダイアログを出す
		 * @return 終了してよい場合true
		 */
		[[nodiscard]] bool confirmQuitGame() noexcept;

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

		/**
		 * @brief デスクトップと全サブウィンドウを引っ込め、ゲーム本体を前面へ戻す
		 *
		 * セレクト画面を抜けるとき（出撃・タイトルへ戻る）に共通で使う。
		 * デスクトップのギミックで開いた実アプリ（cmd.exe等）が前面に残ると、
		 * ボーダーレスのゲーム画面が隠れてしまうため、前面も取り直す
		 */
		void hideAllWindows() noexcept;

		/**
		 * @brief 装備スロットが全て埋まっているかをデスクトップへ伝える
		 *
		 * 3つ選び終えた人が次に何をすればよいか分からず止まってしまうため、
		 * 埋まった時点でデスクトップ側の出撃導線を強調させる。
		 * 外した場合も伝えて、強調を元に戻す
		 */
		void notifyEquipReady() noexcept;

		/**
		 * @brief 設定の変更を受け取ってゲームへ反映する
		 *
		 * デスクトップのクイック設定と設定ウィンドウの両方から届く。
		 * どちらも「変えた項目だけ」を送ってくるので、いまの設定へ混ぜ込む
		 * @param json 届いたJSON文字列（UTF-8）
		 * @return 設定の変更として処理した場合true
		 */
		[[nodiscard]] bool handleSettingsMessage(const std::string& json) noexcept;

		/**
		 * @brief いまの設定をデスクトップと設定ウィンドウへ配る
		 *
		 * 片方で音量を変えたとき、もう片方の表示が置いていかれないようにする
		 */
		void broadcastSettings() noexcept;

		void handleDesktopMessage(const std::string& json) noexcept;
        void notifyWindowState(const std::string& name, bool visible) noexcept;

        std::unique_ptr<DesktopWindow>    m_desktopWindow{};
        std::unique_ptr<FileSelectWindow> m_fileSelectWindow{};
        std::unique_ptr<ParameterWindow>  m_parameterWindow{};
        std::unique_ptr<DifficultyWindow> m_difficultyWindow{};
        std::unique_ptr<RulesWindow>      m_rulesWindow{};
		std::unique_ptr<SettingsWindow> m_settingsWindow{};
		std::unique_ptr<QuickSettingsWindow> m_quickSettingsWindow{};

		bool m_fileVisible{true};
        bool m_paramVisible{true};
        bool m_diffVisible{true};
        bool m_rulesVisible{false};
		bool m_settingsVisible{ false };
		bool m_quickSettingsVisible{ false };

		// 確認ダイアログを出している間だけ回す入力処理と、そのタイマー
		std::function<void(float)> m_modalInputPump{};
		UINT_PTR m_modalPumpTimerId{ 0 };

		// ウィンドウ無しのタイマーは手続きが静的になるため、いま仕掛けている側を控える。
		// セレクト画面は同時に1つしか存在しないので、1つで足りる
		static Win32SelectWindowManager* s_modalPumpOwner;

		// DEBUG: F4での一時退避の状態（リリース時に削除）
		bool m_debugOverlayHidden{ false };
		bool m_debugHideKeyDown{ false };

		// 出撃確認で読み上げるために、選ばれている難易度を控えておく。
		// 初期値は難易度ウィンドウの初期選択（NORMAL）に合わせる
		std::string m_difficulty{ "NORMAL" };

		std::array<std::string, 3> m_slotPaths{};
		std::array<core::data::FileExtensionType, 3> m_slotExtTypes{
			core::data::FileExtensionType::Unknown,
			core::data::FileExtensionType::Unknown,
			core::data::FileExtensionType::Unknown
		};

		std::function<void()> m_onGameStart{};
		std::function<void()> m_onBackToTitle{};
		std::function<void()> m_onQuitGame{};
		std::function<void(int, const std::string&)> m_onFileSlotChanged{};
		std::function<void(const std::string&)> m_onDifficultyChanged{};

		// 設定の正はGame層（SettingsManager）が持つ。ここは読み書きの口だけを預かる
		std::function<core::data::GameSettings()> m_getSettings{};
		std::function<void(const core::data::GameSettings&)> m_onSettingsChanged{};

		core::iface::IResourceManager& m_resourceManager;
        core::iface::IScreen& m_screen;

		// 初回だけ出す操作ガイドを表示するか（ファイル選択ウィンドウへ引き渡す）
		bool m_showTutorial{ false };
	};
} // namespace platform::window::select
