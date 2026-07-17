#include "platform/window/projectile/ProjectileWindowManager.h"
#include "core/interface/ILogger.h"
#include <string>

#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

namespace
{
	// ウィンドウのクライアント全面に描画するロゴ画像のパス
	constexpr const wchar_t* LOGO_IMAGE_PATH{ L"assets/images/ingame/projectile_window.png" };
	// タイトルバー左上のアプリケーションアイコンのパス（.ico）
	constexpr const wchar_t* TITLE_ICON_PATH{ L"assets/images/ui/icons/projectile_window.ico" };
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
		if (Gdiplus::GdiplusStartup(&m_gdiplusToken, &startupInput, nullptr) != Gdiplus::Ok)
		{
			LOG_E("GDI+の初期化に失敗しました（弾ウィンドウのロゴが描画されません）");
			return;
		}

		auto image{ std::make_unique<Gdiplus::Image>(LOGO_IMAGE_PATH) };
		if (image->GetLastStatus() != Gdiplus::Ok)
		{
			LOG_E("弾ウィンドウのロゴ画像の読み込みに失敗しました: assets/images/ingame/projectile_window.png");
			return;
		}
		m_logoImage = std::move(image);

		// タイトルバー用アイコン（.ico）を一度だけ読み込む（全ウィンドウで共有）
		m_titleIcon = static_cast<HICON>(LoadImageW(
		    nullptr, TITLE_ICON_PATH, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE));
		if (m_titleIcon == nullptr)
			LOG_E("弾ウィンドウのアイコン読み込みに失敗しました: assets/images/ui/icons/projectile_window.ico");
	}

	ProjectileWindowManager::~ProjectileWindowManager()
	{
		// GDI+オブジェクト（画像）はGdiplusShutdownより先に破棄する必要がある
		m_slots.clear();
		m_logoImage.reset();
		if (m_gdiplusToken != 0)
			Gdiplus::GdiplusShutdown(m_gdiplusToken);
		if (m_titleIcon != nullptr)
			DestroyIcon(m_titleIcon);
	}

	void ProjectileWindowManager::updateWindows(const std::vector<core::iface::ProjectileWindowInfo>& infos, float deltaTime)
	{
		// DxLibのグラフ座標→実ウィンドウのクライアント座標へのスケールを求める
		// （描画解像度とウィンドウサイズが異なる場合、そのままでは左上へずれるため）
		RECT client{};
		GetClientRect(m_gameWindowHandle, &client);
		const float scaleX{ static_cast<float>(client.right) / static_cast<float>(m_graphWidth) };
		const float scaleY{ static_cast<float>(client.bottom) / static_cast<float>(m_graphHeight) };

		// 今フレーム存在する弾に、同じIDのウィンドウを対応づけて配置する（identity管理）
		const size_t visibleCount{ infos.size() < core::iface::MAX_PROJECTILE_WINDOWS
			                           ? infos.size()
			                           : core::iface::MAX_PROJECTILE_WINDOWS };
		for (size_t i{ 0 }; i < visibleCount; ++i)
		{
			// 既に同じ弾を追従中のスロットがあれば再利用、無ければ空きを割り当てる
			Slot* slot{ findActiveSlot(infos[i].m_projectileId) };
			if (slot == nullptr)
			{
				slot = acquireFreeSlot();
				if (slot == nullptr)
					continue; // 上限に達していて空きが無い
				slot->m_projectileId = infos[i].m_projectileId;
				slot->m_active = true;
			}

			const int size{ static_cast<int>(infos[i].m_size * scaleY) };

			// クライアント座標（ゲーム画面基準）の弾中心をデスクトップ座標へ変換する
			POINT center{
				static_cast<LONG>(infos[i].m_centerX * scaleX),
				static_cast<LONG>(infos[i].m_centerY * scaleY)
			};
			ClientToScreen(m_gameWindowHandle, &center);

			slot->m_window->moveTo(center.x, center.y, size);
		}

		// 今フレームの弾リストに無くなったアクティブスロットはフェードアウトを開始する
		for (auto& slot : m_slots)
		{
			if (!slot.m_active)
				continue;
			bool stillAlive{ false };
			for (size_t i{ 0 }; i < visibleCount; ++i)
			{
				if (infos[i].m_projectileId == slot.m_projectileId)
				{
					stillAlive = true;
					break;
				}
			}
			if (!stillAlive)
			{
				slot.m_active = false;
				slot.m_window->startFadeOut();
			}
		}

		// フェードを進める（完了するとisFading()がfalseに戻り、acquireFreeSlotで再利用される）
		for (auto& slot : m_slots)
			slot.m_window->updateFade(deltaTime);
	}

	void ProjectileWindowManager::hideAll()
	{
		for (auto& slot : m_slots)
		{
			slot.m_window->hide();
			slot.m_active = false;
			slot.m_projectileId = core::ecs::INVALID_ENTITY_ID;
		}
	}

	ProjectileWindowManager::Slot* ProjectileWindowManager::findActiveSlot(core::ecs::EntityId projectileId)
	{
		for (auto& slot : m_slots)
		{
			if (slot.m_active && slot.m_projectileId == projectileId)
				return &slot;
		}
		return nullptr;
	}

	ProjectileWindowManager::Slot* ProjectileWindowManager::acquireFreeSlot()
	{
		// 使用中でもフェード中でもないスロットを再利用する
		for (auto& slot : m_slots)
		{
			if (!slot.m_active && !slot.m_window->isFading())
				return &slot;
		}

		// 空きが無ければ上限まで新規生成する
		if (m_slots.size() >= core::iface::MAX_PROJECTILE_WINDOWS)
			return nullptr;

		// ウィンドウクラス名はインスタンスごとに一意にする（同名だと2個目の登録に失敗する）
		// クラス名はWindowBaseが内部にコピーするため、ローカル文字列でよい
		const std::wstring className{ L"PuraProjectileWindow" + std::to_wstring(m_slots.size()) };
		auto window{ std::make_unique<ProjectileWindow>(className.c_str()) };
		if (!window->createAsProjectile(m_gameWindowHandle))
			return nullptr;
		window->setLogoImage(m_logoImage.get());
		window->setTitleIcon(m_titleIcon);

		Slot slot{};
		slot.m_window = std::move(window);
		m_slots.push_back(std::move(slot));
		return &m_slots.back();
	}
} // namespace platform::window
