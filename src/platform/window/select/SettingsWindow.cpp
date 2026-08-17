#include <windows.h>
#include "SettingsWindow.h"
#include "platform/window/UiSound.h"
#include "platform/window/WindowConstants.h"

namespace platform::window::select
{
	SettingsWindow::SettingsWindow(int x, int y, int width, int height) noexcept
	    : WebViewWindowBase(WINDOW_CLASS_NAME, L"設定", x, y, width, height)
	{
	}

	void SettingsWindow::setOnMessage(std::function<void(const std::string&)> handler) noexcept
	{
		m_onMessage = std::move(handler);
	}

	void SettingsWindow::onCreateControls(HWND hwnd)
	{
		setIcon(hwnd, ICON_PATH);

		m_webView.setOnMessage([this](const std::string& json) noexcept
		    {
			    // 操作音の要求はここで消化する。設定の変更だけを外へ渡す
			    if (platform::window::tryPlayUiSound(json)) return;

			    if (m_onMessage) m_onMessage(json); });

		m_webView.initialize(hwnd, SETTINGS_HTML_URL);
	}

	LRESULT SettingsWindow::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		// サイズ追従・可視追従は WebViewWindowBase に集約している
		if (const auto handled{ handleWebViewMessage(hwnd, msg, wParam, lParam) })
			return *handled;

		return WindowBase::onMessage(hwnd, msg, wParam, lParam);
	}
} // namespace platform::window::select
