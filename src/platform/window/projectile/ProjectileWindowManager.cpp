#include "platform/window/projectile/ProjectileWindowManager.h"
#include <string>

#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

namespace
{
	// ウィンドウのクライアント全面に描画するロゴ画像のパス
	constexpr const wchar_t* LOGO_IMAGE_PATH{ L"assets/images/ingame/projectile_window.png" };
} // namespace

namespace platform::window
{
	ProjectileWindowManager::ProjectileWindowManager(HWND gameWindowHandle, int graphWidth, int graphHeight)
	    : m_gameWindowHandle{ gameWindowHandle }
	    , m_graphWidth{ graphWidth > 0 ? graphWidth : 1 }
	    , m_graphHeight{ graphHeight > 0 ? graphHeight : 1 }
	{
		// GDI+を初期化してロゴ画像を一度だけ読み込む（全ウィンドウで共有する）
		Gdiplus::GdiplusStartupInput startupInput{};
		if (Gdiplus::GdiplusStartup(&m_gdiplusToken, &startupInput, nullptr) == Gdiplus::Ok)
		{
			auto image{ std::make_unique<Gdiplus::Image>(LOGO_IMAGE_PATH) };
			if (image->GetLastStatus() == Gdiplus::Ok)
				m_logoImage = std::move(image);
			// 読み込み失敗時はnullptrのまま＝ウィンドウ側がフォールバック描画する
		}
	}

	ProjectileWindowManager::~ProjectileWindowManager()
	{
		// GDI+オブジェクト（画像）はGdiplusShutdownより先に破棄する必要がある
		m_pool.clear();
		m_logoImage.reset();
		if (m_gdiplusToken != 0)
			Gdiplus::GdiplusShutdown(m_gdiplusToken);
	}

	void ProjectileWindowManager::updateWindows(const std::vector<core::iface::ProjectileWindowInfo>& infos)
	{
		// DxLibのグラフ座標→実ウィンドウのクライアント座標へのスケールを求める
		// （描画解像度とウィンドウサイズが異なる場合、そのままでは左上へずれるため）
		RECT client{};
		GetClientRect(m_gameWindowHandle, &client);
		const float scaleX{ static_cast<float>(client.right) / static_cast<float>(m_graphWidth) };
		const float scaleY{ static_cast<float>(client.bottom) / static_cast<float>(m_graphHeight) };

		const size_t visibleCount{ infos.size() < core::iface::MAX_PROJECTILE_WINDOWS
			                           ? infos.size()
			                           : core::iface::MAX_PROJECTILE_WINDOWS };

		for (size_t i{ 0 }; i < visibleCount; ++i)
		{
			ProjectileWindow* window{ acquireWindow(i) };
			if (window == nullptr)
				continue;

			const int size{ static_cast<int>(infos[i].m_size * scaleY) };

			// クライアント座標（ゲーム画面基準）の弾中心をデスクトップ座標へ変換する
			POINT center{
				static_cast<LONG>(infos[i].m_centerX * scaleX),
				static_cast<LONG>(infos[i].m_centerY * scaleY)
			};
			ClientToScreen(m_gameWindowHandle, &center);

			window->moveTo(center.x, center.y, size);
		}

		// 使っていないウィンドウは隠す
		for (size_t i{ visibleCount }; i < m_pool.size(); ++i)
		{
			if (m_pool[i])
				m_pool[i]->hide();
		}
	}

	void ProjectileWindowManager::hideAll()
	{
		for (auto& window : m_pool)
		{
			if (window)
				window->hide();
		}
	}

	ProjectileWindow* ProjectileWindowManager::acquireWindow(size_t index)
	{
		// プールを必要数まで広げる（生成はここでの1回だけ・以後使い回す）
		while (m_pool.size() <= index)
		{
			// ウィンドウクラス名はインスタンスごとに一意にする（同名だと2個目の登録に失敗する）
			// クラス名はWindowBaseが内部にコピーするため、ローカル文字列でよい
			const std::wstring className{ L"PuraProjectileWindow" + std::to_wstring(m_pool.size()) };
			auto window{ std::make_unique<ProjectileWindow>(className.c_str()) };
			if (!window->createAsProjectile(m_gameWindowHandle))
				return nullptr;
			window->setLogoImage(m_logoImage.get());
			m_pool.push_back(std::move(window));
		}
		return m_pool[index].get();
	}
} // namespace platform::window
