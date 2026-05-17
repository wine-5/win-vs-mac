#include <windows.h>
#include "ParameterWindow.h"
#include "core/data/JobInfo.h"
#include "thirdparty/nlohmann/json.hpp"

namespace platform::window
{
    ParameterWindow::ParameterWindow(int x, int y, int width, int height) noexcept
        : WindowBase(L"ParameterWindowClass", L"Status Display", x, y, width, height)
    {
    }

    void ParameterWindow::refresh(const core::data::JobInfo& jobInfo) noexcept
    {
        if (!m_webView.isReady()) return;

        // Shift-JIS to UTF-8 conversion (nlohmann requires UTF-8)
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
            j["type"]  = "refresh";
            j["name"]  = sjisToUtf8(jobInfo.m_name);
            j["skill"] = sjisToUtf8(jobInfo.m_skillName);
            j["hp"]    = static_cast<int>(jobInfo.m_hp);
            j["atk"]   = static_cast<int>(jobInfo.m_atk);
            j["def"]   = static_cast<int>(jobInfo.m_def);
            j["spd"]   = static_cast<int>(jobInfo.m_spd);
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
            m_webView.resize(LOWORD(lParam), HIWORD(lParam));
            return 0;
        }
        return WindowBase::onMessage(hwnd, msg, wParam, lParam);
    }

}