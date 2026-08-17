#pragma once

#include "platform/window/WebViewWindowBase.h"
#include <functional>
#include <string>

namespace platform::window::select
{
	/**
	 * @class QuickSettingsWindow
	 * @brief タスクバーのトレイから出るクイック設定（Windows の Win+A 相当）
	 *
	 * デスクトップのHTMLの中に描くと、別HWNDであるパラメータ等のウィンドウの背面へ回ってしまう。
	 * z-index はHWNDをまたげないため、独立したウィンドウとして最前面へ出す。
	 * 枠なし（WS_POPUP）にして、OSのポップアップと同じ見た目にしている
	 */
	class QuickSettingsWindow : public platform::window::WebViewWindowBase
	{
	  public:
		/**
		 * @brief コンストラクタ
		 * @param x ウィンドウの左上角 X 座標
		 * @param y ウィンドウの左上角 Y 座標
		 * @param width ウィンドウの幅
		 * @param height ウィンドウの高さ
		 */
		QuickSettingsWindow(int x, int y, int width, int height) noexcept;

		/// @brief デストラクタ
		virtual ~QuickSettingsWindow() noexcept = default;

		/**
		 * @brief WebView から届いた JSON を受け取る処理を登録する
		 * @param handler JSON 文字列を受け取る処理
		 */
		void setOnMessage(std::function<void(const std::string&)> handler) noexcept;

		/**
		 * @brief 兄弟ウィンドウより手前へ出して表示する
		 *
		 * 他のウィンドウを開いた後に呼ばれると背面に回ってしまうため、
		 * 出すたびに毎回z順を取り直す
		 */
		void showOnTop() noexcept;

	  protected:
		void onCreateControls(HWND hwnd) override;
		LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;

	  private:
		static constexpr const wchar_t* WINDOW_CLASS_NAME{ L"QuickSettingsWindowClass" };
		static constexpr const wchar_t* QUICK_HTML_URL{ L"https://game.web/select/quick/quick.html" };

		std::function<void(const std::string&)> m_onMessage{};
	};
} // namespace platform::window::select
