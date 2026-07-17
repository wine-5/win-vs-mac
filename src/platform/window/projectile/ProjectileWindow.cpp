#include "platform/window/projectile/ProjectileWindow.h"

#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

namespace
{
	// 背景色（ウィンドウらしさを出す白）
	constexpr COLORREF BACKGROUND_COLOR{ RGB(255, 255, 255) };

	// タイトルバー付きの通常スタイル（本物のウィンドウらしい見た目）
	constexpr DWORD CHROMED_STYLE{ WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX };
	// 枠なしスタイル（遠くの小さい弾用：タイトルバーを外してロゴを最後まで見せる）
	constexpr DWORD PLAIN_STYLE{ WS_POPUP | WS_BORDER };
	// これよりロゴ領域が小さくなったらタイトルバーを外すしきい値（ピクセル）
	constexpr int CHROME_MIN_SIZE{ 96 };
	// サイズをこの単位に量子化する（毎フレーム1px単位でリサイズ・再描画されるのを防ぐ）
	constexpr int SIZE_QUANTIZATION{ 4 };
} // namespace

namespace platform::window
{
	ProjectileWindow::ProjectileWindow(const wchar_t* className) noexcept
		: WindowBase(className, L"Windows", 0, 0, 64, 64)
	{
		// 本物のウィンドウに見せるため、タイトルバー＋最小化・最大化・閉じるボタンを持たせる
		// （操作は onMessage 側ですべて無効化する）
		m_windowStyle = CHROMED_STYLE;
	}

	bool ProjectileWindow::createAsProjectile(HWND ownerHwnd) noexcept
	{
		if (!create(ownerHwnd))
			return false;

		// フォーカスを奪わない・常に最前面
		// WS_EX_TOOLWINDOW は付けない（細枠になり最小化・最大化ボタンが消えるため）。
		// タスクバー非表示はオーナー付きウィンドウであることで実現される
		const LONG_PTR exStyle{ GetWindowLongPtrW(m_hwnd, GWL_EXSTYLE) };
		SetWindowLongPtrW(m_hwnd, GWL_EXSTYLE,
		    exStyle | WS_EX_NOACTIVATE | WS_EX_TOPMOST);

		// スタイル変更を非クライアント領域（タイトルバー・ボタン）の描画へ反映させる
		SetWindowPos(m_hwnd, nullptr, 0, 0, 0, 0,
		    SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		return true;
	}

	void ProjectileWindow::moveTo(int centerX, int centerY, int size) noexcept
	{
		if (m_hwnd == nullptr)
			return;

		// サイズを量子化し、毎フレームの1px単位のリサイズ・全面再描画を防ぐ
		size -= size % SIZE_QUANTIZATION;
		if (size < SIZE_QUANTIZATION)
			size = SIZE_QUANTIZATION;

		// 小さくなったらタイトルバーを外し、ロゴ（クライアント領域）を最後まで見せる
		const bool wantChrome{ size >= CHROME_MIN_SIZE };
		bool frameChanged{ false };
		if (wantChrome != m_hasChrome)
		{
			SetWindowLongPtrW(m_hwnd, GWL_STYLE,
			    (wantChrome ? CHROMED_STYLE : PLAIN_STYLE) | WS_VISIBLE);
			m_hasChrome = wantChrome;
			frameChanged = true;
		}

		// ロゴ描画領域（クライアント）が size×size になるよう、枠込みのウィンドウサイズを逆算する
		// （タイトルバーはロゴの上に「足される」形になり、ロゴが縮小の犠牲にならない）
		RECT rect{ 0, 0, size, size };
		const DWORD style{ static_cast<DWORD>(GetWindowLongPtrW(m_hwnd, GWL_STYLE)) };
		const DWORD exStyle{ static_cast<DWORD>(GetWindowLongPtrW(m_hwnd, GWL_EXSTYLE)) };
		AdjustWindowRectEx(&rect, style, FALSE, exStyle);

		const int windowWidth{ rect.right - rect.left };
		const int windowHeight{ rect.bottom - rect.top };
		// クライアント領域の中心が弾の中心に一致するよう左上を求める
		// （rect.left / rect.top は枠ぶんだけ負の値になっている）
		const int left{ centerX - size / 2 + rect.left };
		const int top{ centerY - size / 2 + rect.top };

		const bool sizeChanged{ size != m_width };
		m_width = size;
		m_height = size;

		// Zオーダーは拡張スタイルのTOPMOSTで維持されるため毎回は指定しない。
		// 表示指示は非表示のときだけ行う（毎フレームのSHOWWINDOWはコストになる）
		UINT flags{ SWP_NOACTIVATE | SWP_NOZORDER };
		if (!IsWindowVisible(m_hwnd))
			flags |= SWP_SHOWWINDOW;
		if (frameChanged)
			flags |= SWP_FRAMECHANGED;

		SetWindowPos(m_hwnd, nullptr, left, top, windowWidth, windowHeight, flags);

		// サイズ（量子化後）が変わったときだけ再描画する
		if (sizeChanged || frameChanged)
			InvalidateRect(m_hwnd, nullptr, TRUE);
	}

	void ProjectileWindow::setLogoImage(Gdiplus::Image* image) noexcept
	{
		m_logoImage = image;
	}

	LRESULT ProjectileWindow::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		switch (msg)
		{
		case WM_PAINT:
			paintLogo(hwnd);
			return 0;

		// 背景消去はWM_PAINTで塗るため不要（二度塗りによるチラつき・コストを防ぐ）
		case WM_ERASEBKGND:
			return 1;

		// クリックされてもアクティブ化させない（ゲームの操作を邪魔しない）
		case WM_MOUSEACTIVATE:
			return MA_NOACTIVATE;

		// タイトルバー・各ボタンの押下を握りつぶす（ボタンは見えるが反応しない）
		case WM_NCLBUTTONDOWN:
		case WM_NCLBUTTONUP:
		case WM_NCLBUTTONDBLCLK:
		case WM_NCRBUTTONDOWN:
		case WM_NCRBUTTONUP:
			return 0;

		// 万一届いたシステム操作（閉じる・最小化・最大化・移動・サイズ変更）も無効化する
		case WM_SYSCOMMAND:
			return 0;

		// 閉じ要求も無効（弾の寿命はゲーム側が管理する）
		case WM_CLOSE:
			return 0;

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

		// ロゴ画像をクライアント全面に描画する（アルファ付きPNG対応）
		// 画像はマネージャが起動時に読み込み・失敗時はエラーログ済み（ここではnullなら描かない）
		if (m_logoImage != nullptr)
		{
			Gdiplus::Graphics graphics(hdc);
			// 高品質補間は毎フレームの拡縮には重いため通常のバイリニアを使う
			graphics.SetInterpolationMode(Gdiplus::InterpolationModeBilinear);
			graphics.DrawImage(m_logoImage, 0, 0, width, height);
		}

		EndPaint(hwnd, &ps);
	}
} // namespace platform::window
