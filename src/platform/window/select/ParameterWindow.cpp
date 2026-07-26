#include <windows.h>
#include "ParameterWindow.h"
#include "platform/window/WindowConstants.h"
#include "thirdparty/nlohmann/json.hpp"
#include "core/utility/Log.h"
#include <exception>

namespace platform::window::select
{
	ParameterWindow::ParameterWindow(int x, int y, int width, int height) noexcept
	    : WebViewWindowBase(WINDOW_CLASS_NAME, WINDOW_TITLE, x, y, width, height)
	{
    }

	void ParameterWindow::refresh(const ParameterStats& stats) noexcept
	{
		// 準備完了を待たずに送る。WebView2Host が未読み込みの間はキューへ積み、
		// ページの読み込み完了時にまとめて流してくれる。
		// ここで捨てると、画面を開いた直後の初期値が一度も表示されない
		try
		{
            nlohmann::json j;
			j[platform::window::WindowConstants::JSON_KEY_TYPE] = platform::window::WindowConstants::MESSAGE_TYPE_REFRESH;
			j[platform::window::WindowConstants::JSON_KEY_BASE_HP] = stats.m_hp.m_base;
			j[platform::window::WindowConstants::JSON_KEY_BASE_ATK] = stats.m_atk.m_base;
			j[platform::window::WindowConstants::JSON_KEY_BASE_DEF] = stats.m_def.m_base;
			j[platform::window::WindowConstants::JSON_KEY_BASE_SPD] = stats.m_spd.m_base;
			j[platform::window::WindowConstants::JSON_KEY_BONUS_HP] = stats.m_hp.m_bonus;
			j[platform::window::WindowConstants::JSON_KEY_BONUS_ATK] = stats.m_atk.m_bonus;
			j[platform::window::WindowConstants::JSON_KEY_BONUS_DEF] = stats.m_def.m_bonus;
			j[platform::window::WindowConstants::JSON_KEY_BONUS_SPD] = stats.m_spd.m_bonus;
			j[platform::window::WindowConstants::JSON_KEY_BASE_CRIT] = stats.m_crit.m_base;
			j[platform::window::WindowConstants::JSON_KEY_BASE_BSPD] = stats.m_projectileSpeed.m_base;
			j[platform::window::WindowConstants::JSON_KEY_BASE_BRNG] = stats.m_projectileRange.m_base;
			j[platform::window::WindowConstants::JSON_KEY_BONUS_CRIT] = stats.m_crit.m_bonus;
			j[platform::window::WindowConstants::JSON_KEY_BONUS_BSPD] = stats.m_projectileSpeed.m_bonus;
			j[platform::window::WindowConstants::JSON_KEY_BONUS_BRNG] = stats.m_projectileRange.m_bonus;
			j[platform::window::WindowConstants::JSON_KEY_SLOT] = stats.m_equippedSlots;
			m_webView.postMessage(j.dump());
        }
		catch (const std::exception& e)
		{
			core::log::error("ParameterWindow::refresh: 処理に失敗しました: {}", e.what());
		}
		catch (...)
		{
			core::log::error("ParameterWindow::refresh: 不明な例外が発生しました");
		}
	}

    void ParameterWindow::onCreateControls(HWND hwnd)
    {
        setIcon(hwnd, ICON_PATH);
        m_webView.initialize(hwnd, PARAMETER_HTML_URL);
    }

    LRESULT ParameterWindow::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
		// サイズ追従・可視追従は WebViewWindowBase に集約している
		if (const auto handled{ handleWebViewMessage(hwnd, msg, wParam, lParam) })
			return *handled;

		return WindowBase::onMessage(hwnd, msg, wParam, lParam);
    }

} // namespace platform::window::select