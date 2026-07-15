#include <windows.h>
#include "DifficultyWindow.h"
#include "platform/window/WindowConstants.h"
#include "thirdparty/nlohmann/json.hpp"

namespace platform::window::select
{
    DifficultyWindow::DifficultyWindow(int x, int y, int width, int height) noexcept
        : WindowBase(WINDOW_CLASS_NAME, WINDOW_TITLE, x, y, width, height)
    {
    }

    std::string DifficultyWindow::getSelectedDifficulty() const noexcept
    {
        return m_selectedDifficulty;
    }

    void DifficultyWindow::onCreateControls(HWND hwnd)
    {
        setIcon(hwnd, ICON_PATH);
        m_webView.setOnMessage([this](const std::string& json) noexcept {
            handleMessage(json);
        });
        m_webView.initialize(hwnd, DIFFICULTY_HTML_URL);
    }

    LRESULT DifficultyWindow::onMessage(
        HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
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

    void DifficultyWindow::handleMessage(const std::string& json) noexcept
    {
        try
        {
            auto j = nlohmann::json::parse(json);
            const std::string type{ j.value(platform::window::WindowConstants::JSON_KEY_TYPE, "") };

            if (type == MESSAGE_TYPE_DIFFICULTY_CHANGED)
            {
                const std::string diff{ j.value("difficulty", DIFFICULTY_NORMAL) };
                if (diff == DIFFICULTY_EASY || diff == DIFFICULTY_NORMAL || diff == DIFFICULTY_HARD)
                    m_selectedDifficulty = diff;
            }
            else if (type == MESSAGE_TYPE_CONFIRM_HARD)
            {
                const int result{ MessageBoxW(
                    m_hwnd,
                    HARD_WARNING_MESSAGE,
                    HARD_WARNING_TITLE,
                    MB_OKCANCEL | MB_ICONWARNING
                ) };
                if (result == IDOK)
                {
                    m_selectedDifficulty = DIFFICULTY_HARD;
                    nlohmann::json resp;
                    resp[platform::window::WindowConstants::JSON_KEY_TYPE] = MESSAGE_TYPE_HARD_CONFIRMED;
                    m_webView.postMessage(resp.dump());
                }
            }
        }
        catch (...) {}
    }
} // namespace platform::window::select