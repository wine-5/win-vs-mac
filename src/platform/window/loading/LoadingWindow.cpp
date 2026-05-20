#include "LoadingWindow.h"
#include <nlohmann/json.hpp>

namespace platform::window::loading
{
	LoadingWindow::LoadingWindow(int x, int y, int width, int height) noexcept
		: WindowBase(L"LoadingWindow", L"コマンドプロンプト - ローディング中", x, y, width, height)
	{
	}

	void LoadingWindow::setOnLoadingComplete(std::function<void()> callback) noexcept
	{
		m_onLoadingComplete = callback;
	}

	void LoadingWindow::onCreateControls(HWND hwnd)
	{
		setIcon(hwnd, ICON_PATH);

		// WebView2 からのメッセージハンドラを設定（initialize の前に設定）
		m_webView.setOnMessage([this](const std::string& json) {
			handleMessage(json);
		});

		// WebView2 を初期化
		m_webView.initialize(hwnd, L"https://game.web/loading/loading.html");
	}

	LRESULT LoadingWindow::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		return WindowBase::onMessage(hwnd, msg, wParam, lParam);
	}

	void LoadingWindow::handleMessage(const std::string& json) noexcept
	{
		try
		{
			auto data = nlohmann::json::parse(json);

			if (data.contains("type") && data["type"] == "loadingComplete")
			{
				if (m_onLoadingComplete)
					m_onLoadingComplete();
			}
		}
		catch (const std::exception&)
		{
			// JSON パースエラーは無視
		}
	}
}
