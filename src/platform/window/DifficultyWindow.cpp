#include <windows.h>
#include "DifficultyWindow.h"
#include "thirdparty/nlohmann/json.hpp"

namespace platform::window
{
    DifficultyWindow::DifficultyWindow(int x, int y, int width, int height) noexcept
        : WindowBase(L"DifficultyWindowClass", L"Difficulty Selection", x, y, width, height)
    {
    }

    std::string DifficultyWindow::getSelectedDifficulty() const noexcept
    {
        return m_selectedDifficulty;
    }

    void DifficultyWindow::onCreateControls(HWND hwnd)
    {
        m_webView.setOnMessage([this](const std::string& json) noexcept {
            handleMessage(json);
        });
        m_webView.initialize(hwnd, L"https://game.web/difficulty.html");
    }

    LRESULT DifficultyWindow::onMessage(
        HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        if (msg == WM_SIZE)
        {
            m_webView.resize(LOWORD(lParam), HIWORD(lParam));
            return 0;
        }
        return WindowBase::onMessage(hwnd, msg, wParam, lParam);
    }

    void DifficultyWindow::handleMessage(const std::string& json) noexcept
    {
        try
        {
            auto j = nlohmann::json::parse(json);
            const std::string type{ j.value("type", "") };
            if (type == "difficultyChanged")
            {
                const std::string diff{ j.value("difficulty", "NORMAL") };
                if (diff == "EASY" || diff == "NORMAL" || diff == "HARD")
                    m_selectedDifficulty = diff;
            }
        }
        catch (...) {}
    }
}
