#include <windows.h>
#include "RulesWindow.h"

namespace platform::window::select
{
    RulesWindow::RulesWindow(int x, int y, int width, int height) noexcept
        : WindowBase(L"RulesWindowClass", L"readme.txt - \u30e1\u30e2\u5e33", x, y, width, height)
    {
    }

    void RulesWindow::onCreateControls(HWND hwnd)
    {
        setIcon(hwnd, ICON_PATH);
        m_webView.initialize(hwnd, L"https://game.web/select/rules/rules.html");
    }

    LRESULT RulesWindow::onMessage(
        HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        if (msg == WM_SIZE)
        {
            if (wParam != SIZE_MINIMIZED)
                m_webView.resize(LOWORD(lParam), HIWORD(lParam));
            return 0;
        }
        if (msg == WM_SHOWWINDOW)
            m_webView.setVisible(wParam != 0);
        return WindowBase::onMessage(hwnd, msg, wParam, lParam);
    }
}
