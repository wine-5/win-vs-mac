#include <windows.h>
#include "ParameterWindow.h"
#include "thirdparty/nlohmann/json.hpp"

namespace platform::window
{
    ParameterWindow::ParameterWindow(int x, int y, int width, int height) noexcept
        : WindowBase(L"ParameterWindowClass", L"Status Display", x, y, width, height)
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

        auto sjisToUtf8 = [](const std::string& sjis) -> std::string
        {
            if (sjis.empty()) return {};
            int wlen = MultiByteToWideChar(932, 0, sjis.c_str(), -1, nullptr, 0);
            if (wlen <= 0) return {};
            std::wstring wide(wlen - 1, L'\0');
            MultiByteToWideChar(932, 0, sjis.c_str(), -1, wide.data(), wlen);
            int ulen = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
            if (ulen <= 0) return {};
            std::string utf8(ulen - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, utf8.data(), ulen, nullptr, nullptr);
            return utf8;
        };

        try
        {
            nlohmann::json j;
            j["type"]         = "refresh";
            j["job"]          = sjisToUtf8(jobNameSjis);
            j["skill"]        = sjisToUtf8(skillNameSjis);
            j["baseHp"]       = baseHp;
            j["baseAtk"]      = baseAtk;
            j["baseDef"]      = baseDef;
            j["baseSpd"]      = baseSpd;
            j["bonusHp"]      = bonusHp;
            j["bonusAtk"]     = bonusAtk;
            j["bonusDef"]     = bonusDef;
            j["bonusSpd"]     = bonusSpd;
            j["slot"]         = equippedSlots;
            m_webView.postMessage(j.dump());
        }
        catch (...) {}
    }

    void ParameterWindow::onCreateControls(HWND hwnd)
    {
        setIcon(hwnd, ICON_PATH);
        m_webView.initialize(hwnd, L"https://game.web/param/param.html");
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

}