#pragma once
#include <memory>
#include <vector>
#include <windows.h>
#include "core/interface/IProjectileWindowManager.h"
#include "platform/window/projectile/ProjectileWindow.h"

namespace platform::window
{
	/**
	 * @brief 弾追従ウィンドウのプール管理と座標変換を担うマネージャ
	 *
	 * ウィンドウは発射ごとに生成せずプールして使い回す（生成コスト対策）。
	 * ゲームウィンドウのクライアント座標をデスクトップ座標へ変換して配置する。
	 */
	class ProjectileWindowManager : public core::iface::IProjectileWindowManager
	{
	  public:
		/**
		 * @brief ProjectileWindowManagerのコンストラクタ
		 * @param gameWindowHandle ゲームウィンドウのHWND（座標変換・オーナーに使用）
		 */
		explicit ProjectileWindowManager(HWND gameWindowHandle);

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
		std::vector<std::unique_ptr<ProjectileWindow>> m_pool{};
	};
} // namespace platform::window
