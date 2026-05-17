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

        nlohmann::json j;
        j["type"]  = "refresh";
        j["name"]  = jobInfo.m_name;
        j["skill"] = jobInfo.m_skillName;
        j["hp"]    = static_cast<int>(jobInfo.m_hp);
        j["atk"]   = static_cast<int>(jobInfo.m_atk);
        j["def"]   = static_cast<int>(jobInfo.m_def);
        j["spd"]   = static_cast<int>(jobInfo.m_spd);
        m_webView.postMessage(j.dump());
    }

    void ParameterWindow::onCreateControls(HWND hwnd)
    {
        m_webView.initialize(hwnd, L"https://game.web/parameter.html");
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
