#pragma once
#include <vector>
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"

namespace game::system
{
	class PlayerChargeVisualsSystem;
} // namespace game::system

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

		/**
		 * @brief デバッグ可視化の全体ON/OFFを切り替える
		 * @param enabled trueで描画、falseで非描画
		 */
		void setDebugVisualsEnabled(bool enabled);

		/**
		 * @brief 当たり判定（Collider）のデバッグ描画ON/OFFを切り替える
		 * @param enabled trueで描画、falseで非描画
		 */
		void setDebugColliderEnabled(bool enabled);

		/**
		 * @brief 攻撃範囲のデバッグ描画ON/OFFを切り替える
		 * @param enabled trueで描画、falseで非描画
		 */
		void setDebugAttackRangeEnabled(bool enabled);

		/**
		 * @brief 索敵範囲のデバッグ描画ON/OFFを切り替える
		 * @param enabled trueで描画、falseで非描画
		 */
		void setDebugDetectionRangeEnabled(bool enabled);

		/**
		 * @brief 弾（Projectile）の攻撃範囲のデバッグ描画ON/OFFを切り替える
		 * @param enabled trueで描画、falseで非描画
		 */
		void setDebugProjectileRangeEnabled(bool enabled);

		/**
		 * @brief 溜め攻撃の演出System（集中線の描画元）を設定する
		 * @param system PlayerChargeVisualsSystemのポインタ（所有はSystemManager）
		 */
		void setPlayerChargeVisualsSystem(system::PlayerChargeVisualsSystem* system);

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

		/**
		 * @brief DEBUG: 当たり判定（青）を可視化する
		 */
		void drawDebugColliders();

		/**
		 * @brief DEBUG: 攻撃範囲（赤）を可視化する
		 */
		void drawDebugAttackRanges();

		/**
		 * @brief DEBUG: 索敵範囲（黄）を可視化する
		 */
		void drawDebugDetectionRanges();

		/**
		 * @brief DEBUG: 弾（Projectile）の攻撃範囲（赤）を可視化する
		 */
		void drawDebugProjectileRanges();

		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;

		// 溜め攻撃の集中線の描画元（描画内容はSystemが持ち、Viewは描画順だけを管理する）
		// 所有はSystemManagerにあり、InGameがsetupSystemsで設定する
		system::PlayerChargeVisualsSystem* m_playerChargeVisualsSystem{ nullptr };

		bool m_isDebugVisualsEnabled{ true };
		bool m_isDebugColliderEnabled{ false };
		bool m_isDebugAttackRangeEnabled{ true };
		bool m_isDebugDetectionRangeEnabled{ true };
		bool m_isDebugProjectileRangeEnabled{ false };
	};
} // namespace game::scene
