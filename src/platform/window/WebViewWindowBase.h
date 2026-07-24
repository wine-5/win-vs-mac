#pragma once

#include <windows.h>
#include <optional>
#include "platform/window/WindowBase.h"
#include "platform/webview/WebView2Host.h"

namespace platform::window
{
	/**
	 * @brief WebView2 を内包するウィンドウの共通基底
	 *
	 * WebViewを載せたウィンドウは「サイズ追従」「可視状態の追従」を同じ形で必要とする。
	 * 各ウィンドウがWndProcへ書き写すと、修正のたびに全箇所を直す必要があり、
	 * 実際に対応漏れ（WM_ACTIVATEAPPの抜け）が発生していたためここへ集約する。
	 */
	class WebViewWindowBase : public WindowBase
	{
	  public:
		using WindowBase::WindowBase;

	  protected:
		/**
		 * @brief WebViewの標準メッセージ処理（サイズ追従・可視追従）
		 *
		 * 派生クラスの onMessage の先頭で呼び、値が返ってきたらそれをそのまま返すこと。
		 * 値が返らなかった場合は派生固有の処理へ進み、最後に WindowBase::onMessage を返す。
		 * @param hwnd ウィンドウハンドル
		 * @param msg メッセージ
		 * @param wParam wParam
		 * @param lParam lParam
		 * @return 処理を打ち切る場合はその戻り値、そうでなければ nullopt
		 */
		[[nodiscard]] std::optional<LRESULT> handleWebViewMessage(
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

			// 以下は副作用だけ与えて、既定の処理へも流す
			if (msg == WM_SHOWWINDOW)
				m_webView.setVisible(wParam != 0);

			if (msg == WM_ACTIVATEAPP && wParam != 0)
			{
				RECT rc{};
				GetClientRect(hwnd, &rc);
				if (rc.right > 0 && rc.bottom > 0)
					m_webView.resize(rc.right, rc.bottom);
			}

			return std::nullopt;
		}

		platform::webview::WebView2Host m_webView{};
	};
} // namespace platform::window
