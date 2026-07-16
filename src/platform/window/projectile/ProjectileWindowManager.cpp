#include "platform/window/projectile/ProjectileWindowManager.h"
#include <string>

namespace
{
	// 同時に表示する実ウィンドウの上限（超えた分の弾はゲーム内描画のみになる）
	constexpr size_t MAX_WINDOWS{ 10 };
} // namespace

namespace platform::window
{
	ProjectileWindowManager::ProjectileWindowManager(HWND gameWindowHandle)
	    : m_gameWindowHandle{ gameWindowHandle }
	{
	}

	void ProjectileWindowManager::updateWindows(const std::vector<core::iface::ProjectileWindowInfo>& infos)
	{
		const size_t visibleCount{ infos.size() < MAX_WINDOWS ? infos.size() : MAX_WINDOWS };

		for (size_t i{ 0 }; i < visibleCount; ++i)
		{
			ProjectileWindow* window{ acquireWindow(i) };
			if (window == nullptr)
				continue;

			// クライアント座標（ゲーム画面基準）をデスクトップ座標へ変換する
			POINT topLeft{
				infos[i].m_centerX - infos[i].m_size / 2,
				infos[i].m_centerY - infos[i].m_size / 2
			};
			ClientToScreen(m_gameWindowHandle, &topLeft);

			window->moveTo(topLeft.x, topLeft.y, infos[i].m_size);
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
			m_pool.push_back(std::move(window));
		}
		return m_pool[index].get();
	}
} // namespace platform::window
