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
        setIcon(hwnd, ICON_PATH);
        m_webView.setOnMessage([this](const std::string& json) noexcept {
            handleMessage(json);
        });
        m_webView.initialize(hwnd, L"https://game.web/difficulty/difficulty.html");
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
            else if (type == "confirmHard")
            {
                const int result = MessageBoxW(
                    m_hwnd,
                    L"HARD モードを選択しようとしています。\n"
                    L"敵の攻撃力・防御力が大幅に上昇し、\n"
                    L"攻略が非常に困難になります。\n\n"
                    L"本当に続行しますか？",
                    L"警告 - Win vs Mac.exe",
                    MB_OKCANCEL | MB_ICONWARNING
                );
                if (result == IDOK)
                {
                    m_selectedDifficulty = "HARD";
                    m_webView.postMessage(std::string{R"({"type":"hardConfirmed"})"});
                }
            }
        }
        catch (...) {}
    }
}