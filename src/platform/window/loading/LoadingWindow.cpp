#include "LoadingWindow.h"
#include "thirdparty/nlohmann/json.hpp"

namespace platform::window::loading
{
	LoadingWindow::LoadingWindow(int x, int y, int width, int height) noexcept
		: WindowBase(WINDOW_CLASS_NAME, WINDOW_TITLE, x, y, width, height)
	{
	}

	LoadingWindow::~LoadingWindow() noexcept
	{
		destroy();
	}

	void LoadingWindow::setOnLoadingComplete(std::function<void()> callback) noexcept
	{
		m_onLoadingComplete = callback;
	}

	void LoadingWindow::pumpMessages() noexcept
	{
		// メッセージポンプ処理（現在は WebView2 が自動的に処理を行うため不要）
	}

	void LoadingWindow::destroy() noexcept
	{
		WindowBase::destroy();
	}

	void LoadingWindow::onCreateControls(HWND hwnd)
	{
		setIcon(hwnd, ICON_PATH);

		// WebView2 からのメッセージハンドラを設定（initialize の前に設定）
		m_webView.setOnMessage([this](const std::string& json) {
			handleMessage(json);
		});

		// WebView2 を初期化
		m_webView.initialize(hwnd, LOADING_HTML_URL);
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

			if (data.contains(WindowConstants::JSON_KEY_TYPE) &&
				data[WindowConstants::JSON_KEY_TYPE] == WindowConstants::MESSAGE_TYPE_LOADING_COMPLETE)
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
} // namespace platform::window::loading
