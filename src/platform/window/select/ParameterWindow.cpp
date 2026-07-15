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
        const std::string& jobNameSjis,
        const std::string& skillNameSjis,
        int equippedSlots) noexcept
    {
        if (!m_webView.isReady()) return;

        auto sjisToUtf8 = [this](const std::string& sjis) -> std::string
        {
            if (sjis.empty()) return {};
            int wlen = MultiByteToWideChar(SJIS_CODE_PAGE, 0, sjis.c_str(), -1, nullptr, 0);
            if (wlen <= 0) return {};
            std::wstring wide(wlen - 1, L'\0');
            MultiByteToWideChar(SJIS_CODE_PAGE, 0, sjis.c_str(), -1, wide.data(), wlen);
            int ulen{ WideCharToMultiByte(UTF8_CODE_PAGE, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr) };
            if (ulen <= 0) return {};
            std::string utf8(ulen - 1, '\0');
            WideCharToMultiByte(UTF8_CODE_PAGE, 0, wide.c_str(), -1, utf8.data(), ulen, nullptr, nullptr);
            return utf8;
        };

        try
        {
            nlohmann::json j;
            j[platform::window::WindowConstants::JSON_KEY_TYPE]     = platform::window::WindowConstants::MESSAGE_TYPE_REFRESH;
            j[platform::window::WindowConstants::JSON_KEY_JOB]      = sjisToUtf8(jobNameSjis);
            j[platform::window::WindowConstants::JSON_KEY_SKILL]    = sjisToUtf8(skillNameSjis);
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