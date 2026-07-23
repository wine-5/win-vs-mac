#include <windows.h>
#include "ParameterWindow.h"
#include "platform/window/WindowConstants.h"
#include "thirdparty/nlohmann/json.hpp"

namespace platform::window::select
{
	ParameterWindow::ParameterWindow(int x, int y, int width, int height) noexcept
	    : WebViewWindowBase(WINDOW_CLASS_NAME, WINDOW_TITLE, x, y, width, height)
	{
    }

    void ParameterWindow::refresh(
        float baseHp, float baseAtk, float baseDef, float baseSpd,
        float bonusHp, float bonusAtk, float bonusDef, float bonusSpd,
        int equippedSlots) noexcept
    {
        if (!m_webView.isReady()) return;

        try
        {
            nlohmann::json j;
            j[platform::window::WindowConstants::JSON_KEY_TYPE]     = platform::window::WindowConstants::MESSAGE_TYPE_REFRESH;
            j[platform::window::WindowConstants::JSON_KEY_BASE_HP]   = baseHp;
            j[platform::window::WindowConstants::JSON_KEY_BASE_ATK]  = baseAtk;
            j[platform::window::WindowConstants::JSON_KEY_BASE_DEF]  = baseDef;
            j[platform::window::WindowConstants::JSON_KEY_BASE_SPD]  = baseSpd;
            j[platform::window::WindowConstants::JSON_KEY_BONUS_HP]  = bonusHp;
            j[platform::window::WindowConstants::JSON_KEY_BONUS_ATK] = bonusAtk;
            j[platform::window::WindowConstants::JSON_KEY_BONUS_DEF] = bonusDef;
            j[platform::window::WindowConstants::JSON_KEY_BONUS_SPD] = bonusSpd;
            j[platform::window::WindowConstants::JSON_KEY_SLOT]     = equippedSlots;
            m_webView.postMessage(j.dump());
        }
        catch (...) {}
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