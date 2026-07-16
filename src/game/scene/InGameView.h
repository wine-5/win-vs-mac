#pragma once
#include <vector>
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"

namespace game::scene
{
	/**
	 * @brief インゲームの描画を担当するView
	 *
	 * InGame（Scene/コントローラ）から描画の責務を分離する。
	 * モデル描画・照準レティクル（HUD）・デバッグ可視化をまとめて担う。
	 * 状態は持たず、描画に必要なEntityIdを都度受け取って ComponentManager から読み出す。
	 */
	class InGameView
	{
	  public:
		/**
		 * @brief InGameViewのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param renderer 3D描画のインターフェース
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 */
		InGameView(core::ecs::ComponentManager& componentManager,
		           core::iface::IRenderer& renderer,
		           core::iface::IUIRenderer& uiRenderer,
		           core::iface::IScreen& screen);

		/**
		 * @brief インゲームを描画する
		 * @param playerId プレイヤーのEntityID
		 * @param groundId 地面のEntityID
		 * @param enemyIds 敵のEntityID一覧
		 */
		void draw(core::ecs::EntityId playerId,
		          core::ecs::EntityId groundId,
		          const std::vector<core::ecs::EntityId>& enemyIds);

	  private:
		/**
		 * @brief プレイヤー・地面・敵のモデルを描画する
		 */
		void drawModels(core::ecs::EntityId playerId,
		                core::ecs::EntityId groundId,
		                const std::vector<core::ecs::EntityId>& enemyIds);

		/**
		 * @brief 画面中央に照準レティクル（クロスヘア）を描画する
		 * @param playerId 照準状態（AimComponent）を読むプレイヤーのEntityID
		 */
		void drawReticle(core::ecs::EntityId playerId);

		/**
		 * @brief 弾（投射物）を描画する（仮：スフィア。後でビルボード/モデルに差し替え）
		 */
		void drawProjectiles();

		/**
		 * @brief DEBUG: 当たり判定（青）・攻撃範囲（赤）・索敵範囲（黄）を可視化する（テスト後に削除）
		 */
		void drawDebugVisuals();

		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
	};
} // namespace game::scene
