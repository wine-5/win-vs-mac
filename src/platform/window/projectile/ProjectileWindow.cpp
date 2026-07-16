#include "platform/window/projectile/ProjectileWindow.h"

namespace
{
	// ロゴの色（Windows8以降のロゴを思わせるスカイブルー）
	constexpr COLORREF LOGO_COLOR{ RGB(0, 160, 235) };
	// 背景色（ウィンドウらしさを出す白）
	constexpr COLORREF BACKGROUND_COLOR{ RGB(255, 255, 255) };
	// パネル間の隙間がクライアント一辺に占める割合
	constexpr float PANE_GAP_RATIO{ 0.06f };
	// ロゴ全体がクライアント一辺に占める割合（残りは余白）
	constexpr float LOGO_RATIO{ 0.72f };
} // namespace

namespace platform::window
{
	ProjectileWindow::ProjectileWindow(const wchar_t* className) noexcept
	    : WindowBase(className, L"Windows", 0, 0, 64, 64)
	{
		// 枠なしのポップアップ＋細い境界線（タイトルバーなしの飛び道具らしい見た目）
		m_windowStyle = WS_POPUP | WS_BORDER;
	}

	bool ProjectileWindow::createAsProjectile(HWND ownerHwnd) noexcept
	{
		if (!create(ownerHwnd))
			return false;

		// フォーカスを奪わない・タスクバーに出さない・常に最前面
		const LONG_PTR exStyle{ GetWindowLongPtrW(m_hwnd, GWL_EXSTYLE) };
		SetWindowLongPtrW(m_hwnd, GWL_EXSTYLE,
		    exStyle | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST);
		return true;
	}

	void ProjectileWindow::moveTo(int x, int y, int size) noexcept
	{
		if (m_hwnd == nullptr)
			return;

		const bool sizeChanged{ size != m_width };
		m_width = size;
		m_height = size;

		// SWP_NOACTIVATE でフォーカスを奪わずに移動・表示する
		SetWindowPos(m_hwnd, HWND_TOPMOST, x, y, size, size,
		    SWP_NOACTIVATE | SWP_SHOWWINDOW);

		// サイズが変わったときだけ再描画する（毎フレームの無駄な再描画を防ぐ）
		if (sizeChanged)
			InvalidateRect(m_hwnd, nullptr, TRUE);
	}

	LRESULT ProjectileWindow::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		switch (msg)
		{
		case WM_PAINT:
			paintLogo(hwnd);
			return 0;

		// クリックされてもアクティブ化させない（ゲームの操作を邪魔しない）
		case WM_MOUSEACTIVATE:
			return MA_NOACTIVATE;

		default:
			return WindowBase::onMessage(hwnd, msg, wParam, lParam);
		}
	}

	void ProjectileWindow::paintLogo(HWND hwnd) noexcept
	{
		PAINTSTRUCT ps{};
		HDC hdc{ BeginPaint(hwnd, &ps) };

		RECT client{};
		GetClientRect(hwnd, &client);
		const int width{ client.right - client.left };
		const int height{ client.bottom - client.top };

		// 背景を白で塗る（ウィンドウらしさ）
		HBRUSH backBrush{ CreateSolidBrush(BACKGROUND_COLOR) };
		FillRect(hdc, &client, backBrush);
		DeleteObject(backBrush);

		// 中央にWindowsロゴ風の4枚パネル（田の字）を描く
		const int logoSize{ static_cast<int>(width * LOGO_RATIO) };
		const int gap{ static_cast<int>(logoSize * PANE_GAP_RATIO) };
		const int paneSize{ (logoSize - gap) / 2 };
		const int originX{ (width - logoSize) / 2 };
		const int originY{ (height - logoSize) / 2 };

		HBRUSH logoBrush{ CreateSolidBrush(LOGO_COLOR) };
		for (int row{ 0 }; row < 2; ++row)
		{
			for (int col{ 0 }; col < 2; ++col)
			{
				RECT pane{};
				pane.left = originX + col * (paneSize + gap);
				pane.top = originY + row * (paneSize + gap);
				pane.right = pane.left + paneSize;
				pane.bottom = pane.top + paneSize;
				FillRect(hdc, &pane, logoBrush);
			}
		}
		DeleteObject(logoBrush);

		EndPaint(hwnd, &ps);
	}
} // namespace platform::window
