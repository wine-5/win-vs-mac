#include <windows.h>
#pragma comment(lib, "user32.lib")

#ifndef GWL_USERDATA
constexpr int GWL_USERDATA{-21};
#endif

#include "platform/window/WindowBase.h"
#include "core/interface/ILogger.h"

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
            destroy();
        if (m_hIcon != nullptr)
            DestroyIcon(m_hIcon);
    }

    bool WindowBase::create(HWND ownerHwnd) noexcept
    {
        if (m_hwnd != nullptr) return true;

        // RegisterClass してから CreateWindowEx する
        WNDCLASSW wndClass{};
        wndClass.lpfnWndProc = &WindowBase::staticWindowProc;
        wndClass.hInstance = GetModuleHandleW(nullptr);
        wndClass.lpszClassName = m_className.c_str();
        wndClass.hCursor = LoadCursorW(nullptr, (LPCWSTR)IDC_ARROW);
        wndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

        ATOM atom = RegisterClassW(&wndClass);
        if (atom == 0) return false;

        m_hwnd = CreateWindowExW(
            0,
            m_className.c_str(),
            m_title.c_str(),
            WS_OVERLAPPEDWINDOW,
            m_x, m_y, m_width, m_height,
            ownerHwnd,
            nullptr,
            GetModuleHandleW(nullptr),
            this
        );

        if (m_hwnd == nullptr) return false;

        hide();

        return true;
    }

    void WindowBase::destroy() noexcept
    {
        if (m_hwnd == nullptr) return;

        // PostQuitMessage は呼ばない（DxLib のメインループが止まるため）
        if (::DestroyWindow(m_hwnd))
            m_hwnd = nullptr;

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
            ::ShowWindow(m_hwnd, SW_HIDE);
    }

    void WindowBase::bringToFront() noexcept
    {
        if (m_hwnd == nullptr) return;

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

    [[nodiscard]] int WindowBase::getWidth() const noexcept
    {
        return m_width;
    }

    [[nodiscard]] int WindowBase::getHeight() const noexcept
    {
        return m_height;
    }

    LRESULT CALLBACK WindowBase::staticWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        if (msg == WM_CREATE)
        {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            WindowBase* pThis = reinterpret_cast<WindowBase*>(pCreate->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWL_USERDATA, (LONG_PTR)pThis);
            return pThis->onMessage(hwnd, msg, wParam, lParam);
        }

        WindowBase* pThis = reinterpret_cast<WindowBase*>(GetWindowLongPtrW(hwnd, GWL_USERDATA));

        if (pThis != nullptr) return pThis->onMessage(hwnd, msg, wParam, lParam);

        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    void WindowBase::setOnMinimize(std::function<void()> callback) noexcept
    {
        m_onMinimize = std::move(callback);
    }

    void WindowBase::setOnClose(std::function<void()> callback) noexcept
    {
        m_onClose = std::move(callback);
    }

    void WindowBase::setIcon(HWND hwnd, const wchar_t* iconPath) noexcept
    {
        m_hIcon = static_cast<HICON>(LoadImageW(
            nullptr, iconPath, IMAGE_ICON, 0, 0,
            LR_LOADFROMFILE | LR_DEFAULTSIZE
        ));
        if (m_hIcon == nullptr) return;
        SendMessageW(hwnd, WM_SETICON, ICON_BIG,   reinterpret_cast<LPARAM>(m_hIcon));
        SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(m_hIcon));
    }

    LRESULT WindowBase::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        switch (msg)
        {
        case WM_CREATE:
            onCreateControls(hwnd);
            return 0;

        case WM_SYSCOMMAND:
            if ((wParam & 0xFFF0) == SC_MINIMIZE && m_onMinimize)
            {
                m_onMinimize();
                return 0;
            }
            return DefWindowProcW(hwnd, msg, wParam, lParam);

        case WM_CLOSE:
            if (m_onClose)
            {
                m_onClose();
            }
            hide();
            return 0;

        case WM_DESTROY:
            return DefWindowProcW(hwnd, msg, wParam, lParam);

        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
        }
    }
}
