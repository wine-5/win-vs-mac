#pragma once

#include "platform/window/WebViewWindowBase.h"
#include <functional>
#include <string>

namespace platform::window::select
{
	/**
	 * @class SettingsWindow
	 * @brief 音量と操作の設定ウィンドウ（Windows 11「設定」アプリ風）
	 *
	 * 中身はタイトル・ポーズで出る DxLib 側の設定画面と同じ内容にしてある。
	 * 開いた場所で違うものが出ると、同じ「設定」が2つあるように見えてしまう
	 */
	class SettingsWindow : public platform::window::WebViewWindowBase
	{
	  public:
		/**
		 * @brief コンストラクタ
		 * @param x ウィンドウの左上角 X 座標
		 * @param y ウィンドウの左上角 Y 座標
		 * @param width ウィンドウの幅
		 * @param height ウィンドウの高さ
		 */
		SettingsWindow(int x, int y, int width, int height) noexcept;

		/// @brief デストラクタ
		virtual ~SettingsWindow() noexcept = default;

		/**
		 * @brief WebView から届いた JSON を受け取る処理を登録する
		 * @param handler JSON 文字列を受け取る処理
		 */
		void setOnMessage(std::function<void(const std::string&)> handler) noexcept;

	  protected:
		void onCreateControls(HWND hwnd) override;
		LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;

	  private:
		static constexpr const wchar_t* ICON_PATH{ L"assets/images/ui/icons/Settings.ico" };
		static constexpr const wchar_t* WINDOW_CLASS_NAME{ L"SettingsWindowClass" };
		static constexpr const wchar_t* SETTINGS_HTML_URL{ L"https://game.web/select/settings/settings.html" };

		std::function<void(const std::string&)> m_onMessage{};
	};
} // namespace platform::window::select
