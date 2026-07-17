#pragma once
#include <memory>
#include <vector>
#include <windows.h>
#include "core/interface/IProjectileWindowManager.h"
#include "platform/window/projectile/ProjectileWindow.h"

namespace Gdiplus
{
	class Image;
}

namespace platform::window
{
	/**
	 * @brief 弾追従ウィンドウのプール管理と座標変換を担うマネージャ
	 *
	 * ウィンドウは発射ごとに生成せずプールして使い回す（生成コスト対策）。
	 * DxLibの描画解像度（グラフ座標）を実ウィンドウのクライアント座標へスケーリングし、
	 * さらにデスクトップ座標へ変換して配置する。
	 * ロゴ画像（GDI+）はここで一度だけ読み込み、全ウィンドウで共有する。
	 */
	class ProjectileWindowManager : public core::iface::IProjectileWindowManager
	{
	  public:
		/**
		 * @brief ProjectileWindowManagerのコンストラクタ
		 * @param gameWindowHandle ゲームウィンドウのHWND（座標変換・オーナーに使用）
		 * @param graphWidth DxLibの描画解像度の幅（スクリーン座標のスケーリングに使用）
		 * @param graphHeight DxLibの描画解像度の高さ
		 */
		ProjectileWindowManager(HWND gameWindowHandle, int graphWidth, int graphHeight);

		~ProjectileWindowManager() override;

		/**
		 * @brief 今フレームの弾の配置一覧に合わせてウィンドウ群を表示・移動する
		 * @param infos 表示する弾の配置一覧（空なら全ウィンドウを隠す）
		 */
		void updateWindows(const std::vector<core::iface::ProjectileWindowInfo>& infos) override;

		/**
		 * @brief 全ウィンドウを非表示にする
		 */
		void hideAll() override;

	  private:
		/**
		 * @brief プールからindex番目のウィンドウを取得する（未生成なら生成する）
		 * @param index プール内の添字
		 * @return ウィンドウ（生成失敗時nullptr）
		 */
		ProjectileWindow* acquireWindow(size_t index);

		HWND m_gameWindowHandle{ nullptr };
		int m_graphWidth{ 1 };
		int m_graphHeight{ 1 };
		std::vector<std::unique_ptr<ProjectileWindow>> m_pool{};

		ULONG_PTR m_gdiplusToken{ 0 };                 // GDI+の初期化トークン
		std::unique_ptr<Gdiplus::Image> m_logoImage{}; // 全ウィンドウで共有するロゴ画像
		HICON m_titleIcon{ nullptr };                  // 全ウィンドウで共有するタイトルアイコン
	};
} // namespace platform::window
