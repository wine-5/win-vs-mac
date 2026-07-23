#include "Win32SelectWindowManager.h"
#include "core/data/ModelMetadata.h"
#include "game/constant/ModelId.h"
#include "game/constant/MetadataKeys.h"
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
	    core::iface::IResourceManager& resourceManager,
	    core::iface::IScreen& screen) noexcept
	    : m_onGameStart{ std::move(onGameStart) }
	    , m_onFileSlotChanged{ std::move(onFileSlotChanged) }
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
            availH
        );
        if (!m_fileSelectWindow->create(m_desktopWindow->getHwnd())) return;
        m_fileSelectWindow->setOnFileSlotChanged([this](int slot, const std::string& path) noexcept {
            if (m_onFileSlotChanged) m_onFileSlotChanged(slot, path);
            if (slot >= 0 && slot < FILE_SLOT_COUNT)
            {
                m_slotPaths[slot] = path;
				m_slotExtTypes[slot] = game::utility::FileExtensionTypeResolver::fromPath(path);
			}
			updateParameterWindow();
        });
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
    }

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

	void Win32SelectWindowManager::updateParameterWindow() noexcept
	{
        if (!m_parameterWindow) return;

		// 基礎ステータスは playerData.json のメタデータを唯一の情報源とする
		float baseHp{}, baseAtk{}, baseDef{}, baseSpd{};
		if (const auto meta{ m_resourceManager.getMetadata(game::constant::model_id::PLAYER) })
		{
			const auto& props{ meta->floatProperties };
			const auto read{ [&props](std::string_view key, float& out) noexcept
				{
				    if (const auto it{ props.find(std::string{ key }) }; it != props.end())
					    out = it->second;
				} };
			read(game::constant::metadata_keys::MAX_HP, baseHp);
			read(game::constant::metadata_keys::ATTACK_POWER, baseAtk);
			read(game::constant::metadata_keys::DEFENCE, baseDef);
			read(game::constant::metadata_keys::MOVE_SPEED, baseSpd);
		}

		float bonusHp{}, bonusAtk{}, bonusDef{}, bonusSpd{};
        for (int i = 0; i < FILE_SLOT_COUNT; ++i)
        {
            if (!m_slotPaths[i].empty())
            {
                const auto bonus = game::utility::ExtensionBonusCalculator::calculate(m_slotExtTypes[i]);
                bonusHp  += bonus.hp;
                bonusAtk += bonus.atk;
                bonusDef += bonus.def;
                bonusSpd += bonus.spd;
            }
        }

		m_parameterWindow->refresh(
		    baseHp, baseAtk, baseDef, baseSpd,
		    bonusHp, bonusAtk, bonusDef, bonusSpd,
		    countEquippedSlots());
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
