#include <windows.h>
#include "RulesWindow.h"
#include "platform/window/WindowConstants.h"
#include "platform/window/UiSound.h"

namespace platform::window::select
{
	RulesWindow::RulesWindow(int x, int y, int width, int height) noexcept
	    : WebViewWindowBase(L"RulesWindowClass", L"ルール説明.txt - \u30e1\u30e2\u5e33", x, y, width, height)
	{
    }

    void RulesWindow::onCreateControls(HWND hwnd)
    {
        setIcon(hwnd, ICON_PATH);
		// このウィンドウがC++へ送るのは操作音の要求だけ。
		// ページの切り替えはJS内で完結している
		m_webView.setOnMessage([](const std::string& json) noexcept
		    { (void)platform::window::tryPlayUiSound(json); });
		m_webView.initialize(hwnd, RULES_HTML_URL);
    }

    LRESULT RulesWindow::onMessage(
        HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
		// サイズ追従・可視追従は WebViewWindowBase に集約している
		if (const auto handled{ handleWebViewMessage(hwnd, msg, wParam, lParam) })
			return *handled;

		return WindowBase::onMessage(hwnd, msg, wParam, lParam);
    }
} // namespace platform::window::select
