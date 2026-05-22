#include <windows.h>
#include "DesktopWindow.h"

namespace platform::window::select
{
    DesktopWindow::~DesktopWindow() noexcept
    {
        destroy();
    }

    bool DesktopWindow::create(int x, int y, int width, int height) noexcept
    {
        if (m_hwnd) return true;

        WNDCLASSW wc{};
        wc.lpfnWndProc   = staticWindowProc;
        wc.hInstance     = GetModuleHandleW(nullptr);
        wc.lpszClassName = CLASS_NAME;
        wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        RegisterClassW(&wc);

        m_hwnd = CreateWindowExW(
            WS_EX_NOACTIVATE | WS_EX_TOPMOST,
            CLASS_NAME,
            WINDOW_TITLE,
            WS_POPUP,
            x, y, width, height,
            nullptr, nullptr,
            GetModuleHandleW(nullptr),
            this
        );

        if (!m_hwnd) return false;

        m_webView.initialize(m_hwnd, DESKTOP_HTML_URL);
        return true;
    }

    void DesktopWindow::destroy() noexcept
    {
        if (!m_hwnd) return;
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
        UnregisterClassW(CLASS_NAME, GetModuleHandleW(nullptr));
    }

    void DesktopWindow::show() noexcept
    {
        if (m_hwnd)
        {
            ShowWindow(m_hwnd, SW_SHOW);
            UpdateWindow(m_hwnd);
        }
    }

    void DesktopWindow::postMessage(const std::string& utf8Json) noexcept
    {
        m_webView.postMessage(utf8Json);
    }

    void DesktopWindow::setOnMessage(
        std::function<void(const std::string&)> callback) noexcept
    {
        m_webView.setOnMessage(std::move(callback));
    }

    HWND DesktopWindow::getHwnd() const noexcept
    {
        return m_hwnd;
    }

    bool DesktopWindow::isCreated() const noexcept
    {
        return m_hwnd != nullptr;
    }

    LRESULT CALLBACK DesktopWindow::staticWindowProc(
        HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        if (msg == WM_NCCREATE)
        {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        }
        auto* self = reinterpret_cast<DesktopWindow*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (self)
            return self->windowProc(hwnd, msg, wParam, lParam);
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    LRESULT DesktopWindow::windowProc(
        HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        if (msg == WM_SIZE)
        {
            if (wParam != SIZE_MINIMIZED)
                m_webView.resize(LOWORD(lParam), HIWORD(lParam));
            return 0;
        }
        if (msg == WM_ACTIVATEAPP)
        {
            // アプリがアクティブな間だけ TOPMOST にする（DxLib より前面を保証しつつ他アプリへの切り替えも可能）
            SetWindowPos(m_hwnd,
                wParam ? HWND_TOPMOST : HWND_NOTOPMOST,
                0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            if (wParam != 0)
            {
                RECT rc{};
                GetClientRect(hwnd, &rc);
                if (rc.right > 0 && rc.bottom > 0)
                    m_webView.resize(rc.right, rc.bottom);
            }
            return 0;
        }
        if (msg == WM_DESTROY)
        {
            m_hwnd = nullptr;
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}
