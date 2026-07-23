#pragma once
#include <vector>
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IEffectFactory.h"

namespace game::system::visual
{
	class PlayerChargeVisualsSystem;
	class MacAwakenEffectSystem;
	class DetectionAlertVisualsSystem;
	class AttackTelegraphVisualsSystem;
	class TelegraphVisualsSystem;
} // namespace game::system::visual

namespace game::ui::debug
{
	class DebugGizmoView; // DEBUG: 前方宣言（リリース時に削除）
	class DebugHUDView;   // DEBUG: 前方宣言（リリース時に削除）
} // namespace game::ui::debug

namespace game::scene
{
	/**
	 * @brief インゲームの描画を担当するView
	 *
	 * InGame（Scene/コントローラ）から描画の責務を分離する。
	 * モデル描画・照準レティクル（HUD）を担う。
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
		 * @param effectFactory エフェクト（Effekseer）描画のインターフェース
		 */
		InGameView(core::ecs::ComponentManager& componentManager,
		    core::iface::IRenderer& renderer,
		    core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::iface::IEffectFactory& effectFactory);

		/**
		 * @brief インゲームを描画する
		 *
		 * モデルはRenderComponentの全走査で描くため、描画対象のIDは受け取らない。
		 * playerIdはレティクル（照準状態の表示）にのみ使う
		 * @param playerId プレイヤーのEntityID
		 */
		void draw(core::ecs::EntityId playerId);

		/**
		 * @brief 溜め攻撃の演出System（集中線の描画元）を設定する
		 * @param system PlayerChargeVisualsSystemのポインタ（所有はSystemManager）
		 */
		void setPlayerChargeVisualsSystem(system::visual::PlayerChargeVisualsSystem* system);

		/**
		 * @brief ボス覚醒演出System（赤ビネットの描画元）を設定する
		 * @param system MacAwakenEffectSystemのポインタ（所有はSystemManager）
		 */
		void setMacAwakenEffectSystem(system::visual::MacAwakenEffectSystem* system);

		/**
		 * @brief 発見演出System（頭上の通知バッジの描画元）を設定する
		 * @param system DetectionAlertVisualsSystemのポインタ（所有はSystemManager）
		 */
		void setDetectionAlertVisualsSystem(system::visual::DetectionAlertVisualsSystem* system);

		/**
		 * @brief 攻撃予兆System（地面の攻撃範囲サークルの描画元）を設定する
		 * @param system AttackTelegraphVisualsSystemのポインタ（所有はSystemManager）
		 */
		void setAttackTelegraphVisualsSystem(system::visual::AttackTelegraphVisualsSystem* system);

		/**
		 * @brief 汎用攻撃予兆System（TelegraphComponent駆動：円・扇）を設定する
		 * @param system TelegraphVisualsSystemのポインタ（所有はSystemManager）
		 */
		void setTelegraphVisualsSystem(system::visual::TelegraphVisualsSystem* system);

		/**
		 * @brief DEBUG: ワールド空間デバッグ可視化Viewを設定する（リリース時に削除）
		 * @param view DebugGizmoViewのポインタ（所有はInGame）
		 */
		void setDebugGizmoView(ui::debug::DebugGizmoView* view);

		/**
		 * @brief DEBUG: デバッグHUD（FPS等の統計・カメラ状態ラベル）Viewを設定する（リリース時に削除）
		 * @param view DebugHUDViewのポインタ（所有はInGame）
		 */
		void setDebugHUDView(ui::debug::DebugHUDView* view);

	  private:
		/**
		 * @brief RenderComponentを持つ全Entityのモデルを描画する
		 *
		 * 弾は drawProjectileModels が専用に描くため対象外。
		 * 半透明（死亡ディゾルブ中）は不透明の後に描く
		 */
		void drawModels();

		/**
		 * @brief 画面中央に照準レティクル（クロスヘア）を描画する
		 * @param playerId 照準状態（AimComponent）を読むプレイヤーのEntityID
		 */
		void drawReticle(core::ecs::EntityId playerId);

		/**
		 * @brief モデルを持つ弾（Safariのタブ等）を回転させながら描画する
		 *
		 * RenderComponentを持つ弾を対象に、進行方向へyawを向けつつ
		 * 発射地点からの移動距離に応じてタンブル（宙返り）回転させる。
		 */
		void drawProjectileModels();

		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::iface::IEffectFactory& m_effectFactory;

		// 溜め攻撃の集中線の描画元（描画内容はSystemが持ち、Viewは描画順だけを管理する）
		// 所有はSystemManagerにあり、InGameがsetupSystemsで設定する
		system::visual::PlayerChargeVisualsSystem* m_playerChargeVisualsSystem{ nullptr };

		// ボス覚醒演出の赤ビネットの描画元（所有はSystemManager、InGameがsetupSystemsで設定する）
		system::visual::MacAwakenEffectSystem* m_macAwakenEffectSystem{ nullptr };

		// 発見演出（頭上の通知バッジ）の描画元（所有はSystemManager、InGameがsetupSystemsで設定する）
		system::visual::DetectionAlertVisualsSystem* m_detectionAlertSystem{ nullptr };

		// 攻撃予兆（地面の攻撃範囲サークル）の描画元（所有はSystemManager、InGameがsetupSystemsで設定する）
		system::visual::AttackTelegraphVisualsSystem* m_attackTelegraphSystem{ nullptr };
		system::visual::TelegraphVisualsSystem* m_telegraphSystem{ nullptr };

		// DEBUG: デバッグ可視化・HUDの描画元（所有はInGame。リリース時に削除）
		ui::debug::DebugGizmoView* m_debugGizmoView{ nullptr };
		ui::debug::DebugHUDView* m_debugHUDView{ nullptr };
	};
} // namespace game::scene
