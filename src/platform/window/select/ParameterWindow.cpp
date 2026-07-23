#include <windows.h>
#include "ParameterWindow.h"
#include "platform/window/WindowConstants.h"
#include "thirdparty/nlohmann/json.hpp"

namespace platform::window::select
{
    ParameterWindow::ParameterWindow(int x, int y, int width, int height) noexcept
        : WindowBase(WINDOW_CLASS_NAME, WINDOW_TITLE, x, y, width, height)
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
        if (msg == WM_SIZE)
        {
            if (wParam == SIZE_MINIMIZED)
                m_webView.setVisible(false);
            else
                m_webView.resize(LOWORD(lParam), HIWORD(lParam));
            return 0;
        }
        if (msg == WM_SHOWWINDOW)
            m_webView.setVisible(wParam != 0);
        if (msg == WM_ACTIVATEAPP && wParam != 0)
        {
            RECT rc{};
            GetClientRect(hwnd, &rc);
            if (rc.right > 0 && rc.bottom > 0)
                m_webView.resize(rc.right, rc.bottom);
        }
        return WindowBase::onMessage(hwnd, msg, wParam, lParam);
    }

} // namespace platform::window::select