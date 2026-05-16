#include <windows.h>
#pragma comment(lib, "user32.lib")

#ifndef GWL_USERDATA
#define GWL_USERDATA (-21)
#endif

#include "WindowBase.h"

namespace platform::window
{
    WindowBase::WindowBase(const wchar_t* className, const wchar_t* title,
        int x, int y, int width, int height) noexcept
        : m_className(className), m_title(title), m_x(x), m_y(y),
        m_width(width), m_height(height), m_hwnd(nullptr)
    {
    }

    WindowBase::~WindowBase() noexcept
    {
        if (m_hwnd != nullptr)
        {
            destroy();
        }
    }

    bool WindowBase::create() noexcept
    {
        if (m_hwnd != nullptr)
        {
            return true;
        }

        // RegisterClass してから CreateWindowEx する
        WNDCLASSW wndClass{};
        wndClass.lpfnWndProc = &WindowBase::staticWindowProc;
        wndClass.hInstance = GetModuleHandleW(nullptr);
        wndClass.lpszClassName = m_className.c_str();
        wndClass.hCursor = LoadCursorW(nullptr, (LPCWSTR)IDC_ARROW);
        wndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

        ATOM atom = RegisterClassW(&wndClass);
        if (atom == 0)
        {
            return false;
        }

        m_hwnd = CreateWindowExW(
            0,
            m_className.c_str(),
            m_title.c_str(),
            WS_OVERLAPPEDWINDOW,
            m_x, m_y, m_width, m_height,
            nullptr,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr
        );

        if (m_hwnd == nullptr)
        {
            return false;
        }

        // static 関数の staticWindowProc から this を復元できるようにする
        SetWindowLongPtrW(m_hwnd, GWL_USERDATA, (LONG_PTR)this);

        hide();

        return true;
    }

    void WindowBase::destroy() noexcept
    {
        if (m_hwnd == nullptr)
        {
            return;
        }

        // PostQuitMessage は呼ばない（DxLib のメインループが止まるため）
        if (::DestroyWindow(m_hwnd))
        {
            m_hwnd = nullptr;
        }

        UnregisterClassW(m_className.c_str(), GetModuleHandleW(nullptr));
    }

    void WindowBase::show() noexcept
    {
        if (m_hwnd != nullptr)
        {
            ::ShowWindow(m_hwnd, SW_SHOW);
            ::UpdateWindow(m_hwnd);
        }
    }

    void WindowBase::hide() noexcept
    {
        if (m_hwnd != nullptr)
        {
            ::ShowWindow(m_hwnd, SW_HIDE);
        }
    }

    void WindowBase::bringToFront() noexcept
    {
        if (m_hwnd == nullptr)
        {
            return;
        }

        ::SetForegroundWindow(m_hwnd);
        SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    }

    [[nodiscard]] HWND WindowBase::getHwnd() const noexcept
    {
        return m_hwnd;
    }

    [[nodiscard]] bool WindowBase::isCreated() const noexcept
    {
        return m_hwnd != nullptr && ::IsWindow(m_hwnd);
    }

    LRESULT CALLBACK WindowBase::staticWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        WindowBase* pThis = reinterpret_cast<WindowBase*>(GetWindowLongPtrW(hwnd, GWL_USERDATA));

        if (pThis != nullptr)
        {
            return pThis->onMessage(hwnd, msg, wParam, lParam);
        }

        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    LRESULT WindowBase::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        switch (msg)
        {
        case WM_CREATE:
            onCreateControls(hwnd);
            return 0;

        case WM_CLOSE:
            hide();
            return 0;

        case WM_DESTROY:
            return DefWindowProcW(hwnd, msg, wParam, lParam);

        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
        }
    }
}
