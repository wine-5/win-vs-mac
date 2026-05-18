#include <windows.h>
#include "ResultWindow.h"
#include "thirdparty/nlohmann/json.hpp"
#include "core/base/ServiceLocator.h"
#include "game/scene/SceneManager.h"
#include "game/scene/SceneType.h"

namespace platform::window::result
{
    ResultWindow::ResultWindow(
        core::iface::IScreen& screen,
        std::function<void()> onRetry,
        std::function<void()> onTitle) noexcept
        : WindowBase(L"ResultWindowClass", L"Result - Win vs Mac.exe",
            0, 0, WINDOW_WIDTH, WINDOW_HEIGHT)
        , m_screen{ screen }
        , m_onRetry{ std::move(onRetry) }
        , m_onTitle{ std::move(onTitle) }
    {
    }

    void ResultWindow::show(const core::data::ResultData& data) noexcept
    {
        m_pendingData = data;

        if (!isCreated())
        {
            HWND dxlibHwnd{ static_cast<HWND>(m_screen.getNativeWindowHandle()) };

            // タイトルバー・ボーダーなしのポップアップウィンドウにする
            m_windowStyle = WS_POPUP;

            // DxLib ウィンドウのクライアント領域（ゲーム描画部分）の位置・サイズを取得
            RECT cr{};
            GetClientRect(dxlibHwnd, &cr);
            POINT pt{ cr.left, cr.top };
            ClientToScreen(dxlibHwnd, &pt);
            m_x      = pt.x;
            m_y      = pt.y;
            m_width  = cr.right  - cr.left;
            m_height = cr.bottom - cr.top;

            create(dxlibHwnd);
        }

        WindowBase::show();
    }

    void ResultWindow::pumpMessages() noexcept
    {
        MSG msg{};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void ResultWindow::onCreateControls(HWND hwnd)
    {
        m_webView.setOnMessage([this](const std::string& json) noexcept {
            handleMessage(json);
        });
        m_webView.initialize(hwnd, L"https://game.web/result/result.html");
    }

    LRESULT ResultWindow::onMessage(
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

    void ResultWindow::handleMessage(const std::string& json) noexcept
    {
        try
        {
            auto j{ nlohmann::json::parse(json) };
            const std::string type{ j.value("type", "") };

            if (type == "requestResult")
            {
                if (m_pendingData.has_value())
                    sendResultData(m_pendingData.value());
            }
            else if (type == "retry")
            {
                // シーン遷移前にウィンドウを非表示にする
                hide();
                if (m_onRetry)
                    m_onRetry();
            }
            else if (type == "title")
            {
                // シーン遷移前にウィンドウを非表示にする
                hide();
                if (m_onTitle)
                    m_onTitle();
            }
        }
        catch (...) {}
    }

    void ResultWindow::sendResultData(const core::data::ResultData& data) noexcept
    {
        try
        {
            nlohmann::json j{};
            j["type"]             = "resultData";
            j["isVictory"]        = data.m_isVictory;
            j["elapsedTime"]      = data.m_elapsedTime;
            j["killCount"]        = data.m_killCount;
            j["totalDamageTaken"] = data.m_totalDamageTaken;
            j["usedFiles"]        = data.m_usedFiles;

            m_webView.postMessage(j.dump());
        }
        catch (...) {}
    }
}
