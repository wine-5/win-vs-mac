#include <windows.h>
#include "QuickSettingsWindow.h"
#include "platform/window/UiSound.h"

namespace platform::window::select
{
	QuickSettingsWindow::QuickSettingsWindow(int x, int y, int width, int height) noexcept
	    : WebViewWindowBase(WINDOW_CLASS_NAME, L"", x, y, width, height)
	{
		// タイトルバーも枠も持たないポップアップにする。
		// OSのクイック設定に枠が無いのと、閉じるボタンを押させる画面ではないため
		m_windowStyle = WS_POPUP;
	}

	void QuickSettingsWindow::setOnMessage(std::function<void(const std::string&)> handler) noexcept
	{
		m_onMessage = std::move(handler);
	}

	void QuickSettingsWindow::showOnTop() noexcept
	{
		if (!getHwnd())
			return;

		// 表示は必ず ShowWindow で行う。SetWindowPos の SWP_SHOWWINDOW では
		// WM_SHOWWINDOW が飛ばず、WebView 側が非表示のままになって白紙で出てしまう
		show();

		// 兄弟の中で最前面へ移す。SWP_NOACTIVATE を付けるのは、
		// ここへフォーカスを奪うとデスクトップ側のクリック判定が途切れるため
		::SetWindowPos(getHwnd(), HWND_TOP, 0, 0, 0, 0,
		    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	void QuickSettingsWindow::onCreateControls(HWND hwnd)
	{
		m_webView.setOnMessage([this](const std::string& json) noexcept
		    {
			    // 操作音の要求はここで消化する。設定の変更だけを外へ渡す
			    if (platform::window::tryPlayUiSound(json)) return;

			    if (m_onMessage) m_onMessage(json); });

		m_webView.initialize(hwnd, QUICK_HTML_URL);
	}

	LRESULT QuickSettingsWindow::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		// サイズ追従・可視追従は WebViewWindowBase に集約している
		if (const auto handled{ handleWebViewMessage(hwnd, msg, wParam, lParam) })
			return *handled;

		return WindowBase::onMessage(hwnd, msg, wParam, lParam);
	}
} // namespace platform::window::select
