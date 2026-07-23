#pragma once

#include <windows.h>
#include <functional>
#include "platform/window/WebViewWindowBase.h"
#include "platform/window/WindowConstants.h"
#include "core/interface/IWindow.h"

namespace platform::window::loading
{
	/**
	 * @class LoadingWindow
	 * @brief ローディングウィンドウ
	 */
	class LoadingWindow
	    : public platform::window::WebViewWindowBase,
	      public core::iface::IWindow
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
		virtual ~LoadingWindow() noexcept;

		/**
		 * @brief ローディング完了時のコールバック設定
		 * @param callback ローディング完了時に呼ぶコールバック関数
		 */
		void setOnLoadingComplete(std::function<void()> callback) noexcept;

		/**
		 * @brief メッセージポンプ（毎フレーム呼び出し）
		 */
		void pumpMessages() noexcept override;

		/**
		 * @brief ウィンドウを破棄する
		 */
		void destroy() noexcept override;

	  protected:
		void onCreateControls(HWND hwnd) override;
		LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;

	  private:
		// ウィンドウ定数
		static constexpr const wchar_t* WINDOW_CLASS_NAME{ L"LoadingWindow" };
		static constexpr const wchar_t* WINDOW_TITLE{ L"コマンドプロンプト - ローディング中" };
		static constexpr const wchar_t* ICON_PATH{ L"assets/images/ui/icons/cmdicon.ico" };
		static constexpr const wchar_t* LOADING_HTML_URL{ L"https://game.web/loading/loading.html" };

		std::function<void()> m_onLoadingComplete{};

		void handleMessage(const std::string& json) noexcept;
	};
} // namespace platform::window::loading
