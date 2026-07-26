#include "Win32SelectWindowManager.h"
#include "core/data/ModelMetadata.h"
#include "game/constant/ModelId.h"
#include "game/constant/MetadataKeys.h"
#include "game/constant/ProjectileId.h"
#include "core/data/ProjectileMetadata.h"
#include "platform/window/WindowConstants.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "platform/utility/StringConverter.h"
#include "thirdparty/nlohmann/json.hpp"
#include <shellapi.h>
#include <windows.h>
#include "core/utility/Log.h"
#include <exception>
#include <utility>

namespace platform::window::select
{
	Win32SelectWindowManager::Win32SelectWindowManager(
	    std::function<void()> onGameStart,
	    std::function<void(int, const std::string&)> onFileSlotChanged,
	    std::function<void(const std::string&)> onDifficultyChanged,
	    core::iface::IResourceManager& resourceManager,
	    core::iface::IScreen& screen) noexcept
	    : m_onGameStart{ std::move(onGameStart) }
	    , m_onFileSlotChanged{ std::move(onFileSlotChanged) }
	    , m_onDifficultyChanged{ std::move(onDifficultyChanged) }
	    , m_resourceManager{ resourceManager }
	    , m_screen{ screen }
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

        // DxLibクライアント領域全体を覆うDesktopWindow
        m_desktopWindow = std::make_unique<DesktopWindow>();
        m_desktopWindow->setOnMessage([this](const std::string& json) noexcept {
            handleDesktopMessage(json);
        });
        if (!m_desktopWindow->create(originX, originY, screenWidth, screenHeight)) return;
        m_desktopWindow->show();

		// --- レイアウト定数 ---
		// 左列: 上はデスクトップアイコンを見せるため空け、下に難易度パネル
		// 中央列: File  /  右列: Parameter
		int marginX{ screenWidth  * MARGIN_PERCENT / 100 };
        int marginY{ screenHeight * MARGIN_PERCENT / 100 };
        int colWidth{ (screenWidth  - marginX * COLUMN_COUNT) / COLUMN_COUNT };
        int availH{    screenHeight - TASKBAR_HEIGHT - marginY * 2 };
		// 左列の上側はデスクトップアイコンを見せるために空ける。難易度パネルはその下に置く
		int iconAreaH{ availH * ICON_AREA_RATIO / ICON_AREA_RATIO_BASE };
		int diffH{ availH - iconAreaH - GAP_Y };

		int winY{ originY + marginY };

        // FileSelectウィンドウ（中央列 全高）
		m_fileSelectWindow = std::make_unique<FileSelectWindow>(
		    originX + marginX * 2 + colWidth,
		    winY,
		    colWidth,
		    availH,
		    m_resourceManager);
		if (!m_fileSelectWindow->create(m_desktopWindow->getHwnd())) return;
		// noexcept にしない。文字列の代入などで例外が出た場合、noexcept だと
		// std::terminate になってログも残らず即死する。
		// ここで投げれば FileSelectWindow::handleMessage の catch がログに残す
		m_fileSelectWindow->setOnFileSlotChanged([this](int slot, const std::string& path)
		    {
            if (m_onFileSlotChanged) m_onFileSlotChanged(slot, path);
            if (slot >= 0 && slot < FILE_SLOT_COUNT)
            {
                m_slotPaths[slot] = path;
				m_slotExtTypes[slot] = game::utility::FileExtensionTypeResolver::fromPath(path);
			}
			updateParameterWindow(); });
		m_fileSelectWindow->setOnMinimize([this]() noexcept {
            m_fileSelectWindow->hide();
            m_fileVisible = false;
            notifyWindowState(WINDOW_NAME_FILE, false);
        });
        m_fileSelectWindow->setOnClose([this]() noexcept {
            m_fileVisible = false;
            notifyWindowState(WINDOW_NAME_FILE, false);
        });

        // Parameterウィンドウ（右列 全高）
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
            notifyWindowState(WINDOW_NAME_PARAM, false);
        });
        m_parameterWindow->setOnClose([this]() noexcept {
            m_paramVisible = false;
            notifyWindowState(WINDOW_NAME_PARAM, false);
        });

		// DifficultyWindow（左列下。上側はデスクトップアイコンのため空けている）
		m_difficultyWindow = std::make_unique<DifficultyWindow>(
		    originX + marginX,
		    winY + iconAreaH + GAP_Y,
		    colWidth,
		    diffH);
		if (!m_difficultyWindow->create(m_desktopWindow->getHwnd())) return;
		m_difficultyWindow->setOnDifficultyChanged([this](const std::string& difficulty) noexcept
		    {
            if (m_onDifficultyChanged) m_onDifficultyChanged(difficulty); });
		m_difficultyWindow->setOnMinimize([this]() noexcept {
            m_difficultyWindow->hide();
            m_diffVisible = false;
            notifyWindowState(WINDOW_NAME_DIFF, false);
        });
        m_difficultyWindow->setOnClose([this]() noexcept {
            m_diffVisible = false;
            notifyWindowState(WINDOW_NAME_DIFF, false);
        });

        // RulesWindow（センタリング・初期非表示）
        m_rulesWindow = std::make_unique<RulesWindow>(
            originX + (screenWidth  - RULES_WINDOW_WIDTH) / 2,
            originY + (screenHeight - RULES_WINDOW_HEIGHT) / 2,
            RULES_WINDOW_WIDTH,
            RULES_WINDOW_HEIGHT
        );
        if (!m_rulesWindow->create(m_desktopWindow->getHwnd())) return;
        m_rulesWindow->setOnMinimize([this]() noexcept {
            m_rulesWindow->hide();
            m_rulesVisible = false;
            notifyWindowState(WINDOW_NAME_RULES, false);
        });
        m_rulesWindow->setOnClose([this]() noexcept {
            m_rulesVisible = false;
            notifyWindowState(WINDOW_NAME_RULES, false);
        });

        m_fileSelectWindow->setAlpha(WINDOW_ALPHA);
        m_parameterWindow->setAlpha(WINDOW_ALPHA);
        m_difficultyWindow->setAlpha(WINDOW_ALPHA);
        m_rulesWindow->setAlpha(WINDOW_ALPHA);

        m_fileSelectWindow->show();
        m_parameterWindow->show();
        m_difficultyWindow->show();

		// 何も装備していない状態の基礎値を最初から見せる。
		// ファイルを1つ選ぶまで全項目が「—」のままだと、何が伸びるのか比較できない
		updateParameterWindow();
	}

    void Win32SelectWindowManager::destroyAllWindows()
    {
        if (m_fileSelectWindow) m_fileSelectWindow->destroy();
        if (m_parameterWindow)  m_parameterWindow->destroy();
        if (m_difficultyWindow) m_difficultyWindow->destroy();
        if (m_rulesWindow)      m_rulesWindow->destroy();
        if (m_desktopWindow)    m_desktopWindow->destroy();

        m_fileSelectWindow.reset();
        m_parameterWindow.reset();
        m_difficultyWindow.reset();
        m_rulesWindow.reset();
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

		updateDebugOverlayToggle(); // DEBUG: リリース時に削除
	}

	// DEBUG: ここからセレクト画面の一時退避（リリース時に削除する）

	void Win32SelectWindowManager::updateDebugOverlayToggle() noexcept
	{
#ifdef _DEBUG
		// セレクト画面のデスクトップは常時最前面なので、その裏にあるコンソールや
		// 例外ダイアログを読むことができない。F4で一時的に引っ込められるようにする。
		// GetAsyncKeyState は押しっぱなしでも真になるため、押した瞬間だけを拾う
		const bool isDown{ (GetAsyncKeyState(DEBUG_HIDE_KEY) & 0x8000) != 0 };
		const bool isPressed{ isDown && !m_debugHideKeyDown };
		m_debugHideKeyDown = isDown;

		if (!isPressed)
			return;

		m_debugOverlayHidden = !m_debugOverlayHidden;

		// 子ウィンドウは親（デスクトップ）を隠せば一緒に消える
		if (m_desktopWindow && m_desktopWindow->getHwnd())
			ShowWindow(m_desktopWindow->getHwnd(), m_debugOverlayHidden ? SW_HIDE : SW_SHOW);

		core::log::info("DEBUG: セレクト画面の表示を{}にしました",
		    m_debugOverlayHidden ? "非表示" : "表示");
#endif
	}

	// DEBUG: ここまで

	int Win32SelectWindowManager::countEquippedSlots() const noexcept
	{
		int count{};
		for (int i = 0; i < FILE_SLOT_COUNT; ++i)
		{
			if (!m_slotPaths[i].empty())
				++count;
		}
		return count;
	}

	void Win32SelectWindowManager::updateParameterWindow()
	{
        if (!m_parameterWindow) return;

		ParameterStats stats{};

		// 基礎ステータスは playerData.json のメタデータを唯一の情報源とする
		if (const auto meta{ m_resourceManager.getMetadata(game::constant::model_id::PLAYER) })
		{
			const auto& props{ meta->floatProperties };
			const auto read{ [&props](std::string_view key, float& out) noexcept
				{
				    if (const auto it{ props.find(std::string{ key }) }; it != props.end())
					    out = it->second;
				} };
			read(game::constant::metadata_keys::MAX_HP, stats.m_hp.m_base);
			read(game::constant::metadata_keys::ATTACK_POWER, stats.m_atk.m_base);
			read(game::constant::metadata_keys::DEFENCE, stats.m_def.m_base);
			read(game::constant::metadata_keys::MOVE_SPEED, stats.m_spd.m_base);
			read(game::constant::metadata_keys::ATTACK_RANGE, stats.m_attackRange.m_base);

			// 会心率は0.0〜1.0の確率で持っているが、そのままでは0.2などと出て読みにくい。
			// 表示だけ%へ直す（ボーナス側も同じ倍率を掛ける）
			float criticalRate{};
			read(game::constant::metadata_keys::CRITICAL_RATE, criticalRate);
			stats.m_crit.m_base = criticalRate * PERCENT_SCALE;
		}

		// Window弾の基礎値は playerData.json ではなく projectileData.json 側が持つ。
		// 飛距離は定義に無いため「弾速×寿命」で求める。
		// getProjectileMetadata は弾IDが無いと例外を投げる。この関数は noexcept なので、
		// 素通しすると std::terminate になりゲームごと落ちる。必ずここで受け止める
		try
		{
			const auto& projectileMeta{ m_resourceManager.getProjectileMetadata(
				game::constant::projectile_id::PLAYER_WINDOW) };
			stats.m_projectileSpeed.m_base = projectileMeta.m_speed;
			stats.m_projectileRange.m_base = projectileMeta.m_speed * projectileMeta.m_lifetime;
		}
		catch (const std::exception& e)
		{
			core::log::error("updateParameterWindow: Window弾の定義を取得できませんでした: {}", e.what());
		}

		for (int i = 0; i < FILE_SLOT_COUNT; ++i)
        {
            if (!m_slotPaths[i].empty())
            {
				const auto& bonus = m_resourceManager.getExtensionBonus(m_slotExtTypes[i]);
				stats.m_hp.m_bonus += bonus.hp;
				stats.m_atk.m_bonus += bonus.atk;
				stats.m_def.m_bonus += bonus.def;
				stats.m_spd.m_bonus += bonus.spd;
				stats.m_attackRange.m_bonus += bonus.attackRange;
				stats.m_crit.m_bonus += bonus.criticalRate * PERCENT_SCALE;
				stats.m_projectileSpeed.m_bonus += bonus.projectileSpeed;
				stats.m_projectileRange.m_bonus += bonus.projectileRange;
			}
        }

		stats.m_equippedSlots = countEquippedSlots();
		m_parameterWindow->refresh(stats);
	}

    void Win32SelectWindowManager::handleDesktopMessage(const std::string& json) noexcept
    {
        try
        {
            auto j = nlohmann::json::parse(json);
            const std::string type{ j.value(platform::window::WindowConstants::JSON_KEY_TYPE, "") };

            if (type == platform::window::WindowConstants::MESSAGE_TYPE_START_GAME)
            {
				// ファイル装備は任意。ただし埋まっていないスロットがある場合は確認を挟む
				if (countEquippedSlots() < FILE_SLOT_COUNT && !confirmStartWithEmptySlots())
					return;

				// ゲーム開始前に全サブウィンドウを非表示にしてからコールバックを実行
                if (m_desktopWindow && m_desktopWindow->getHwnd())
                    ShowWindow(m_desktopWindow->getHwnd(), SW_HIDE);
                if (m_fileSelectWindow) m_fileSelectWindow->hide();
                if (m_parameterWindow)  m_parameterWindow->hide();
                if (m_difficultyWindow) m_difficultyWindow->hide();

				// デスクトップのギミックで開いた実アプリ（cmd.exe等）が前面に残ると、
				// ボーダーレスのゲーム画面が隠れてしまう。ゲーム本体ウィンドウを前面へ戻す
				if (HWND gameHwnd{ static_cast<HWND>(m_screen.getNativeWindowHandle()) })
				{
					SetForegroundWindow(gameHwnd);
					SetActiveWindow(gameHwnd);
				}

				if (m_onGameStart) m_onGameStart();
            }
            else if (type == platform::window::WindowConstants::MESSAGE_TYPE_TOGGLE_WINDOW)
            {
                const std::string name{ j.value(platform::window::WindowConstants::JSON_KEY_WINDOW, "") };
				if (name == WINDOW_NAME_FILE && m_fileSelectWindow)
				{
                    m_fileVisible = !m_fileVisible;
                    m_fileVisible ? m_fileSelectWindow->show() : m_fileSelectWindow->hide();
                    notifyWindowState(WINDOW_NAME_FILE, m_fileVisible);
                }
                else if (name == WINDOW_NAME_PARAM && m_parameterWindow)
                {
                    m_paramVisible = !m_paramVisible;
                    m_paramVisible ? m_parameterWindow->show() : m_parameterWindow->hide();
                    notifyWindowState(WINDOW_NAME_PARAM, m_paramVisible);
                }
                else if (name == WINDOW_NAME_DIFF && m_difficultyWindow)
                {
                    m_diffVisible = !m_diffVisible;
                    m_diffVisible ? m_difficultyWindow->show() : m_difficultyWindow->hide();
                    notifyWindowState(WINDOW_NAME_DIFF, m_diffVisible);
                }
                else if (name == WINDOW_NAME_RULES && m_rulesWindow)
                {
                    m_rulesVisible = !m_rulesVisible;
                    m_rulesVisible ? m_rulesWindow->show() : m_rulesWindow->hide();
                    notifyWindowState(WINDOW_NAME_RULES, m_rulesVisible);
                }
            }
            else if (type == platform::window::WindowConstants::MESSAGE_TYPE_LAUNCH_APP)
            {
                const std::string app{ j.value(platform::window::WindowConstants::JSON_KEY_APP, "") };

				// デスクトップ背景ウィンドウは常時最前面(TOPMOST)なので、そのままだと
				// 起動したアプリがその後ろに隠れてしまう（TOPMOSTはフォーカスに関係なくz順で手前）。
				// 起動時に最前面指定を解除して、実アプリが前に出られるようにする。
				// ゲームへ戻れば WM_ACTIVATEAPP で再びTOPMOSTに戻る
				if (m_desktopWindow && m_desktopWindow->getHwnd())
					SetWindowPos(m_desktopWindow->getHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0,
					    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

				// Windowsのフォーカス保護で後ろに開くのを防ぎ、起動アプリが自分で前面に出られるようにする
				AllowSetForegroundWindow(ASFW_ANY);

				if (app == "cmd")
                    ShellExecuteW(nullptr, L"open", APP_CMD_PATH, nullptr, nullptr, SW_SHOW);
                else if (app == "taskmgr")
                    ShellExecuteW(nullptr, L"open", APP_TASKMGR_PATH, nullptr, nullptr, SW_SHOW);
                else if (app == "recyclebin")
                    ShellExecuteW(nullptr, L"open", APP_RECYCLEBIN_PATH, nullptr, nullptr, SW_SHOW);
                else if (app == "notepad")
                    ShellExecuteW(nullptr, L"open", APP_NOTEPAD_PATH, nullptr, nullptr, SW_SHOW);
            }
        }
		catch (const std::exception& e)
		{
			core::log::error("Win32SelectWindowManager::handleDesktopMessage: 処理に失敗しました: {}", e.what());
		}
		catch (...)
		{
			core::log::error("Win32SelectWindowManager::handleDesktopMessage: 不明な例外が発生しました");
		}
	}

    void Win32SelectWindowManager::notifyWindowState(
        const std::string& name, bool visible) noexcept
    {
        if (!m_desktopWindow) return;
        try
        {
            nlohmann::json j;
            j[platform::window::WindowConstants::JSON_KEY_TYPE]    = platform::window::WindowConstants::MESSAGE_TYPE_WINDOW_STATE_CHANGED;
            j[platform::window::WindowConstants::JSON_KEY_WINDOW]  = name;
            j[platform::window::WindowConstants::JSON_KEY_VISIBLE] = visible;
            m_desktopWindow->postMessage(j.dump());
        }
		catch (const std::exception& e)
		{
			core::log::error("Win32SelectWindowManager::notifyWindowState: 処理に失敗しました: {}", e.what());
		}
		catch (...)
		{
			core::log::error("Win32SelectWindowManager::notifyWindowState: 不明な例外が発生しました");
		}
	}

    void Win32SelectWindowManager::showWarningMessage(const std::string& message) noexcept
    {
        HWND parentHwnd = (m_desktopWindow && m_desktopWindow->getHwnd()) ? m_desktopWindow->getHwnd() : nullptr;

        platform::utility::StringConverter converter;
        std::wstring wMessage = converter.utf8ToWide(message);
        MessageBoxW(parentHwnd, wMessage.c_str(), L"警告", MB_OK | MB_ICONWARNING);
    }

	bool Win32SelectWindowManager::confirmStartWithEmptySlots() noexcept
	{
		HWND parentHwnd = (m_desktopWindow && m_desktopWindow->getHwnd()) ? m_desktopWindow->getHwnd() : nullptr;

		platform::utility::StringConverter converter;
		const std::wstring message{ converter.utf8ToWide(
			"装備ファイルが3つ選択されていません。\n"
			"ボーナスを受け取らずにこのまま開始しますか？") };

		return MessageBoxW(parentHwnd, message.c_str(), L"確認",
		           MB_OKCANCEL | MB_ICONWARNING | MB_DEFBUTTON2) == IDOK;
	}
} // namespace platform::window::select
