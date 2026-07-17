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
		 * @param deltaTime フレーム間の時間差（消滅時のフェードアウトに使う）
		 */
		void updateWindows(const std::vector<core::iface::ProjectileWindowInfo>& infos, float deltaTime) override;

		/**
		 * @brief 全ウィンドウを非表示にする
		 */
		void hideAll() override;

	  private:
		/**
		 * @brief 弾ウィンドウ1枚分の枠。どの弾(EntityId)に割り当てられているかを保持する
		 */
		struct Slot
		{
			std::unique_ptr<ProjectileWindow> m_window{};
			core::ecs::EntityId m_projectileId{ core::ecs::INVALID_ENTITY_ID };
			bool m_active{ false }; // この弾に追従中か（falseならフェード中or空き）
		};

		/**
		 * @brief 指定した弾IDに割り当て済みのアクティブなスロットを探す
		 * @param projectileId 弾のEntityId
		 * @return 見つかったスロット（無ければnullptr）
		 */
		Slot* findActiveSlot(core::ecs::EntityId projectileId);

		/**
		 * @brief 割り当て可能な空きスロットを取得する（無ければ生成する）
		 * @return 空きスロット（上限到達で生成不可ならnullptr）
		 */
		Slot* acquireFreeSlot();

		HWND m_gameWindowHandle{ nullptr };
		int m_graphWidth{ 1 };
		int m_graphHeight{ 1 };
		std::vector<Slot> m_slots{};

		ULONG_PTR m_gdiplusToken{ 0 };                 // GDI+の初期化トークン
		std::unique_ptr<Gdiplus::Image> m_logoImage{}; // 全ウィンドウで共有するロゴ画像
		HICON m_titleIcon{ nullptr };                  // 全ウィンドウで共有するタイトルアイコン
	};
} // namespace platform::window
