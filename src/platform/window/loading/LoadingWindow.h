#pragma once

#include <windows.h>
#include <functional>
#include "platform/window/WindowBase.h"
#include "platform/webview/WebView2Host.h"

namespace platform::window::loading
{
	/**
	 * @class LoadingWindow
	 * @brief ローディングウィンドウ
	 */
	class LoadingWindow : public WindowBase
	{
	public:
		/**
		 * @brief コンストラクタ
		 * @param x ウィンドウの左上角 X 座標
		 * @param y ウィンドウの左上角 Y 座標
		 * @param width ウィンドウの幅
		 * @param height ウィンドウの高さ
		 */
		LoadingWindow(int x, int y, int width, int height) noexcept;

		/// @brief デストラクタ
		virtual ~LoadingWindow() noexcept = default;

		/**
		 * @brief ローディング完了時のコールバック設定
		 * @param callback ローディング完了時に呼ぶコールバック関数
		 */
		void setOnLoadingComplete(std::function<void()> callback) noexcept;

	protected:
		void onCreateControls(HWND hwnd) override;
		LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;

	private:
		static constexpr const wchar_t* ICON_PATH = L"assets/images/ui/icons/loading.ico";

		platform::webview::WebView2Host m_webView{};
		std::function<void()> m_onLoadingComplete{};

		void handleMessage(const std::string& json) noexcept;
	};
}
