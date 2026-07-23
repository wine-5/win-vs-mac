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
        : m_className{className}, m_title{title}, m_x{x}, m_y{y},
        m_width{width}, m_height{height}, m_hwnd{nullptr}
    {
    }

    WindowBase::~WindowBase() noexcept
    {
        // HWND が生きていれば WM_DESTROY まで処理してからハンドルを破棄する
        if (m_hwnd != nullptr)
            destroy();
        // アイコンは HWND と独立したリソースなので別途解放する
        if (m_hIcon != nullptr)
            DestroyIcon(m_hIcon);
    }

    bool WindowBase::create(HWND ownerHwnd) noexcept
    {
        if (m_hwnd != nullptr) return true;

        // ウィンドウクラスを登録してからウィンドウを生成する
        WNDCLASSW wndClass{};
        wndClass.lpfnWndProc = &WindowBase::staticWindowProc;
        wndClass.hInstance = GetModuleHandleW(nullptr);
        wndClass.lpszClassName = m_className.c_str();
        wndClass.hCursor = LoadCursorW(nullptr, (LPCWSTR)IDC_ARROW);
        wndClass.hbrBackground = (HBRUSH)BACKGROUND_BRUSH_COLOR;

        ATOM atom = RegisterClassW(&wndClass);
        if (atom == 0) return false;

        // lpCreateParams に this を渡すことで staticWindowProc 内から
        // WM_CREATE のタイミングにインスタンスポインタを GWL_USERDATA に保存できる
        m_hwnd = CreateWindowExW(
            0,
            m_className.c_str(),
            m_title.c_str(),
            m_windowStyle,
            m_x, m_y, m_width, m_height,
            ownerHwnd,
            nullptr,
            GetModuleHandleW(nullptr),
            this
        );

        if (m_hwnd == nullptr) return false;

        // 生成直後は非表示にしておき、呼び出し側が show() で制御する
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

    void WindowBase::setAlpha(BYTE alpha) noexcept
    {
        if (m_hwnd == nullptr) return;
        // SetLayeredWindowAttributes を使うには WS_EX_LAYERED が必要なので既存スタイルに OR で付与する
        LONG_PTR exStyle{ GetWindowLongPtrW(m_hwnd, GWL_EXSTYLE) };
        SetWindowLongPtrW(m_hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
        // LWA_ALPHA でアルファ値による透過（カラーキーではなく一様透過）を指定する
        SetLayeredWindowAttributes(m_hwnd, 0, alpha, LWA_ALPHA);
    }

    LRESULT CALLBACK WindowBase::staticWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        if (msg == WM_CREATE)
        {
            // WM_CREATE 時だけ lpCreateParams からインスタンスポインタを取り出して
            // GWL_USERDATA に保存する。以降のメッセージでは GWL_USERDATA から復元する
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
        // LR_LOADFROMFILE でファイルから読み込み、LR_DEFAULTSIZE でシステム既定サイズを使用する
        m_hIcon = static_cast<HICON>(LoadImageW(
            nullptr, iconPath, IMAGE_ICON, 0, 0,
            LR_LOADFROMFILE | LR_DEFAULTSIZE
        ));
        if (m_hIcon == nullptr) return;
        // 大アイコン（タスクバー等）と小アイコン（タイトルバー等）の両方に設定する
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
            // 下位 4 ビットはマウス座標などの付加情報なのでマスクして SC_MINIMIZE と比較する
            if ((wParam & SYSCOMMAND_MASK) == SC_MINIMIZE && m_onMinimize)
            {
                m_onMinimize();
                return 0; // デフォルトの最小化を抑制する
            }
            return DefWindowProcW(hwnd, msg, wParam, lParam);

        case WM_CLOSE:
            if (m_onClose)
                m_onClose();
                
            // DestroyWindow は呼ばず hide() で非表示にするだけにする
            // （ゲーム側でウィンドウを再表示できるようにするため）
            hide();
            return 0;

        case WM_DESTROY:
            return DefWindowProcW(hwnd, msg, wParam, lParam);

        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
        }
    }
} // namespace platform::window
