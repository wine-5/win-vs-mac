#pragma once
#include <vector>
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IEffectFactory.h"
#include "core/interface/IShadowMap.h"
#include "game/component/visual/RenderComponent.h"
#include "game/system/visual/HudDropSystem.h"
#include <functional>

namespace game::system::combat
{
	class PlayerDeathSystem;
	class PlayerRangedAttackSystem;
} // namespace game::system::combat

namespace game::system::visual
{
	class PlayerChargeVisualsSystem;
	class CriticalVisualsSystem;
	class MacAwakenEffectSystem;
	class DetectionAlertVisualsSystem;
	class AttackTelegraphVisualsSystem;
	class TelegraphVisualsSystem;
	class BackgroundParticleSystem;
	class HardAuraVisualsSystem;
	class RimLightVisualsSystem;
	class ShockwaveVisualsSystem;
	class DamagePopupSystem;
	class BattleStartSystem;
} // namespace game::system::visual

namespace game::ui::debug
{
	class DebugGizmoView; // DEBUG: 前方宣言（リリース時に削除）
	class DebugHUDView;   // DEBUG: 前方宣言（リリース時に削除）
} // namespace game::ui::debug

namespace game::ui::ingame
{
	class PlayerHUDView;     // 前方宣言
	class EquipmentSlotView; // 前方宣言
	class ObjectiveView;     // 前方宣言
	class InGameStatusView;  // 前方宣言
	class InventoryView;     // 前方宣言
	class InteractPromptView;    // 前方宣言
	class LowHealthVignetteView;
	class ExtensionBoostFlashView; // 前方宣言
	class BossHUDView;           // 前方宣言
	class MiniMapView;           // 前方宣言
	class EnemyHealthBarView;    // 前方宣言
} // namespace game::ui::ingame

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
		 * @param shadowMap 影の描き分けのインターフェース
		 */
		InGameView(core::ecs::ComponentManager& componentManager,
		    core::iface::IRenderer& renderer,
		    core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::iface::IEffectFactory& effectFactory,
		    core::iface::IShadowMap& shadowMap);

		/**
		 * @brief インゲームを描画する
		 *
		 * モデルはRenderComponentの全走査で描くため、描画対象のIDは受け取らない。
		 * playerIdはレティクル（照準状態の表示）とプレイヤーHUDに使う
		 * @param playerId プレイヤーのEntityID
		 * @param remainingEnemyCount 残っている討伐対象の雑魚の数（左上の目標表示に使う）
		 * @param bossId ボスのEntityID（未出現ならINVALID_ENTITY_ID）
		 * @param elapsedTime インゲーム開始からの経過時間（秒。右上の状況表示に使う）
		 */
		void draw(core::ecs::EntityId playerId, int remainingEnemyCount, core::ecs::EntityId bossId,
		    float elapsedTime);

		/**
		 * @brief 溜め攻撃の演出System（集中線の描画元）を設定する
		 * @param system PlayerChargeVisualsSystemのポインタ（所有はSystemManager）
		 */
		void setPlayerChargeVisualsSystem(system::visual::PlayerChargeVisualsSystem* system);

		/**
		 * @brief クリティカルの演出System（弾ける集中線の描画元）を設定する
		 * @param system CriticalVisualsSystemのポインタ（所有はSystemManager）
		 */
		void setCriticalVisualsSystem(system::visual::CriticalVisualsSystem* system);

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
		 * @brief ダメージ数値の表示System（敵に与えた数値の描画元）を設定する
		 * @param system DamagePopupSystemのポインタ（所有はSystemManager）
		 */
		void setDamagePopupSystem(system::visual::DamagePopupSystem* system);

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
		 * @brief 背景パーティクル（虚空を流れるデータの光跡）Systemを設定する
		 * @param system BackgroundParticleSystemのポインタ（所有はSystemManager）
		 */
		void setBackgroundParticleSystem(system::visual::BackgroundParticleSystem* system);

		/**
		 * @brief Hardの敵を包む赤いオーラのSystemを設定する
		 * @param system HardAuraVisualsSystemのポインタ（所有はSystemManager）
		 */
		void setHardAuraVisualsSystem(system::visual::HardAuraVisualsSystem* system);

		/**
		 * @brief 輪郭光のSystemを設定する
		 * @param system RimLightVisualsSystemのポインタ（所有はSystemManager）
		 */
		void setRimLightVisualsSystem(system::visual::RimLightVisualsSystem* system);

		/**
		 * @brief 地面を走る衝撃波のSystemを設定する
		 * @param system ShockwaveVisualsSystemのポインタ（所有はSystemManager）
		 */
		void setShockwaveVisualsSystem(system::visual::ShockwaveVisualsSystem* system);

		/**
		 * @brief HUD落下のSystemを設定する
		 * @param system HudDropSystemのポインタ（所有はSystemManager）
		 */
		void setHudDropSystem(system::visual::HudDropSystem* system);

		/**
		 * @brief 開始演出System（READY / FIGHT! の描画元）を設定する
		 * @param system BattleStartSystemのポインタ（所有はSystemManager）
		 */
		void setBattleStartSystem(system::visual::BattleStartSystem* system);

		/**
		 * @brief プレイヤー死亡演出System（暗転の描画元）を設定する
		 * @param system PlayerDeathSystemのポインタ（所有はSystemManager）
		 */
		void setPlayerDeathSystem(system::combat::PlayerDeathSystem* system);

		/**
		 * @brief 遠隔攻撃System（クールダウン残量の取得元）を設定する
		 * @param system PlayerRangedAttackSystemのポインタ（所有はSystemManager）
		 */
		void setPlayerRangedAttackSystem(system::combat::PlayerRangedAttackSystem* system);

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

		/**
		 * @brief プレイヤーステータス（左下のHUD）Viewを設定する
		 * @param view PlayerHUDViewのポインタ（所有はInGame）
		 */
		void setPlayerHUDView(ui::ingame::PlayerHUDView* view);

		/**
		 * @brief 装備スロット（右下のHUD）Viewを設定する
		 * @param view EquipmentSlotViewのポインタ（所有はInGame）
		 */
		void setEquipmentSlotView(ui::ingame::EquipmentSlotView* view);

		/**
		 * @brief 目標表示（左上のHUD）Viewを設定する
		 * @param view ObjectiveViewのポインタ（所有はInGame）
		 */
		void setObjectiveView(ui::ingame::ObjectiveView* view);

		/**
		 * @brief 状況表示（右上のHUD：難易度・経過時間）Viewを設定する
		 * @param view InGameStatusViewのポインタ（所有はInGame）
		 */
		void setInGameStatusView(ui::ingame::InGameStatusView* view);

		/**
		 * @brief 拡張子インベントリ（Eキー）のViewを設定する
		 * @param view InventoryViewのポインタ（所有はInGame）
		 */
		void setInventoryView(ui::ingame::InventoryView* view);

		/**
		 * @brief インベントリの開閉状態を設定する
		 *
		 * 開いている間だけ描く。閉じているときに描くと画面を覆ってしまう
		 * @param isOpen 開いているならtrue
		 */
		void setInventoryOpen(bool isOpen);

		/**
		 * @brief 設置物への接近案内（吹き出し）Viewを設定する
		 * @param view InteractPromptViewのポインタ（所有はInGame）
		 */
		void setInteractPromptView(ui::ingame::InteractPromptView* view);

		/**
		 * @brief 案内を出す対象のEntityIDを設定する
		 * @param targetId 対象のEntityID（範囲内に無ければ INVALID_ENTITY_ID）
		 */
		void setInteractTarget(core::ecs::EntityId targetId);

		/**
		 * @brief 低HP警告のビネットViewを設定する
		 * @param view LowHealthVignetteViewのポインタ（所有はInGame）
		 */
		void setLowHealthVignetteView(ui::ingame::LowHealthVignetteView* view);

		/**
		 * @brief ギャンブルボックスの当たりを知らせるViewを設定する
		 * @param view 設定するView（所有はInGame側）
		 */
		void setExtensionBoostFlashView(ui::ingame::ExtensionBoostFlashView* view);

		/**
		 * @brief ボスHP（上中央のHUD）Viewを設定する
		 * @param view BossHUDViewのポインタ（所有はInGame）
		 */
		void setBossHUDView(ui::ingame::BossHUDView* view);

		/**
		 * @brief ミニマップのViewを設定する
		 * @param view MiniMapViewのポインタ（所有はInGame）
		 */
		void setMiniMapView(ui::ingame::MiniMapView* view);

		/**
		 * @brief 敵の頭上HPバーViewを設定する
		 * @param view EnemyHealthBarViewのポインタ（所有はInGame）
		 */
		void setEnemyHealthBarView(ui::ingame::EnemyHealthBarView* view);

	  private:
		/**
		 * @brief RenderComponentを持つ全Entityのモデルを描画する
		 *
		 * 弾は drawProjectileModels が専用に描くため対象外。
		 * 半透明（死亡ディゾルブ中）は不透明の後に描く
		 */
		void drawModels();

		/**
		 * @brief 影を落とすEntityをシャドウマップへ描画する
		 *
		 * ShadowCasterComponent を持つEntityだけが対象。影の写る範囲は
		 * プレイヤーの周囲に限っているため、そこから外れたものは描かずに飛ばす。
		 * @param playerId 影の範囲の中心にするプレイヤーのEntityId
		 */
		void drawShadowCasters(core::ecs::EntityId playerId);

		/**
		 * @brief 落下対象のHUDを、落下ぶんずらして描く
		 *
		 * 落下していない間もこの経路を通すことで、演出の有無による分岐を
		 * 各HUDの描画箇所へ書かずに済ませる。
		 * @param slot 対象のHUD
		 * @param drawBody 実際の描画処理
		 */
		void drawDroppableHud(system::visual::HudSlot slot, const std::function<void()>& drawBody);

		/**
		 * @brief 影を落とすEntityの水平方向の広がり（半分）を返す
		 *
		 * 影の範囲へ完全に収まっているかの判定に使う。中心座標だけで見ると、
		 * 端が範囲からはみ出した物を描いてしまい、シャドウマップの縁が
		 * 範囲外へ引き伸ばされて巨大な偽の影が出る。
		 * @param entityId 対象のEntityId
		 * @param render 対象のRenderComponent
		 * @return 中心からの水平方向の広がり。分からない場合は0
		 */
		[[nodiscard]] float castingHalfExtent(core::ecs::EntityId entityId,
		    const component::visual::RenderComponent& render) const;

		/**
		 * @brief Entityが装着している武器を、装着先ボーンへ追従させて描画する
		 *
		 * 本体モデルを描いた直後に呼ぶこと（ボーンのワールド行列が確定するため）。
		 * 装着先ボーンの解決は WeaponAttachSystem が済ませている前提で、
		 * 未解決のものは描かない
		 * @param entityId 装着元EntityのID
		 */
		void drawAttachedWeapon(core::ecs::EntityId entityId);

		/**
		 * @brief 画面中央に照準レティクル（クロスヘア）を描画する
		 * @param playerId 照準状態（AimComponent）を読むプレイヤーのEntityID
		 */
		void drawReticle(core::ecs::EntityId playerId);

		/**
		 * @brief 溜め攻撃の進行度をレティクルの外周に描画する
		 *
		 * 円弧を描くプリミティブが無いため、外周上に等間隔で並べた点を
		 * 溜め率のぶんだけ点灯させて進行度を示す。溜め中のみ表示する
		 * @param playerId 溜め状態（PlayerChargeComponent）を読むプレイヤーのEntityID
		 * @param centerX レティクル中心のX座標
		 * @param centerY レティクル中心のY座標
		 * @param radius 点を並べる円の半径（レティクル外周と揃える）
		 */
		void drawChargeGauge(core::ecs::EntityId playerId, int centerX, int centerY, int radius);

		/**
		 * @brief 攻撃のクールダウン残量を返す
		 *
		 * 近接と遠隔で別々に出すと中央が賑やかになるため、残りの長いほうへ一本化する
		 * @param playerId 近接の状態（AttackComponent）を読むプレイヤーのEntityID
		 * @return 0.0（撃てる）〜1.0（撃った直後）
		 */
		[[nodiscard]] float getAttackCooldownRatio(core::ecs::EntityId playerId) const;

		/**
		 * @brief モデルを持つ弾（Safariのタブ等）を回転させながら描画する
		 *
		 * RenderComponentを持つ弾を対象に、進行方向へyawを向けつつ
		 * 発射地点からの移動距離に応じてタンブル（宙返り）回転させる。
		 */
		void drawProjectileModels();

		/**
		 * @brief 落ちている拡張子の欠片を描画する
		 *
		 * 装備スロットと同じアイコンを、光るビルボードとして浮かせて描く。
		 * 発光させるのは「拾えるもの」だと一目で分かるようにするため。
		 */
		void drawExtensionPickups();

		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::iface::IEffectFactory& m_effectFactory;
		core::iface::IShadowMap& m_shadowMap;

		// 溜め攻撃の集中線の描画元（描画内容はSystemが持ち、Viewは描画順だけを管理する）
		// 所有はSystemManagerにあり、InGameがsetupSystemsで設定する
		system::visual::PlayerChargeVisualsSystem* m_playerChargeVisualsSystem{ nullptr };

		// クリティカルの集中線の描画元（所有はSystemManager、InGameがsetupSystemsで設定する）
		system::visual::CriticalVisualsSystem* m_criticalVisualsSystem{ nullptr };

		// ボス覚醒演出の赤ビネットの描画元（所有はSystemManager、InGameがsetupSystemsで設定する）
		system::visual::MacAwakenEffectSystem* m_macAwakenEffectSystem{ nullptr };

		// 発見演出（頭上の通知バッジ）の描画元（所有はSystemManager、InGameがsetupSystemsで設定する）
		system::visual::DetectionAlertVisualsSystem* m_detectionAlertSystem{ nullptr };

		// ダメージ数値の描画元（所有はSystemManager、InGameがsetupSystemsで設定する）
		system::visual::DamagePopupSystem* m_damagePopupSystem{ nullptr };

		// 攻撃予兆（地面の攻撃範囲サークル）の描画元（所有はSystemManager、InGameがsetupSystemsで設定する）
		system::visual::AttackTelegraphVisualsSystem* m_attackTelegraphSystem{ nullptr };
		system::visual::TelegraphVisualsSystem* m_telegraphSystem{ nullptr };

		// 背景パーティクルの描画元（所有はSystemManager、InGameがsetupSystemsで設定する）
		system::visual::BackgroundParticleSystem* m_backgroundParticleSystem{ nullptr };

		// Hardの敵を包む赤いオーラの描画元（所有はSystemManager、InGameがsetupSystemsで設定する）
		system::visual::HardAuraVisualsSystem* m_hardAuraVisualsSystem{ nullptr };
		system::visual::RimLightVisualsSystem* m_rimLightVisualsSystem{ nullptr };
		system::visual::ShockwaveVisualsSystem* m_shockwaveVisualsSystem{ nullptr };
		system::visual::HudDropSystem* m_hudDropSystem{ nullptr };

		// プレイヤーステータス（左下のHUD）の描画元（所有はInGame）
		ui::ingame::PlayerHUDView* m_playerHUDView{ nullptr };

		// 装備スロット（右下のHUD）の描画元（所有はInGame）
		ui::ingame::EquipmentSlotView* m_equipmentSlotView{ nullptr };

		// 目標表示（左上のHUD）の描画元（所有はInGame）
		ui::ingame::ObjectiveView* m_objectiveView{ nullptr };

		// 状況表示（右上のHUD：難易度・経過時間）の描画元（所有はInGame）
		ui::ingame::InGameStatusView* m_statusView{ nullptr };

		// 拡張子インベントリ（Eキー）の描画元（所有はInGame）。開いている間だけ描く
		ui::ingame::InventoryView* m_inventoryView{ nullptr };
		bool m_isInventoryOpen{ false };

		// 設置物への接近案内（吹き出し）の描画元（所有はInGame）
		ui::ingame::InteractPromptView* m_interactPromptView{ nullptr };
		core::ecs::EntityId m_interactTargetId{ core::ecs::INVALID_ENTITY_ID };

		// 低HP警告のビネットの描画元（所有はInGame）
		ui::ingame::LowHealthVignetteView* m_lowHealthVignetteView{ nullptr };

		/// @brief 当たりを引いた瞬間の閃光とメッセージ
		ui::ingame::ExtensionBoostFlashView* m_extensionBoostFlashView{ nullptr };

		// ボスHP（上中央のHUD）の描画元（所有はInGame）
		ui::ingame::BossHUDView* m_bossHUDView{ nullptr };
		ui::ingame::MiniMapView* m_miniMapView{ nullptr };

		// 敵の頭上HPバーの描画元（所有はInGame）
		ui::ingame::EnemyHealthBarView* m_enemyHealthBarView{ nullptr };

		// 開始演出（READY / FIGHT!）の描画元（所有はSystemManager、InGameがsetupSystemsで設定する）
		system::visual::BattleStartSystem* m_battleStartSystem{ nullptr };

		// プレイヤー死亡時の暗転の描画元（所有はSystemManager、InGameがsetupSystemsで設定する）
		system::combat::PlayerDeathSystem* m_playerDeathSystem{ nullptr };

		// 遠隔攻撃のクールダウン残量の取得元（所有はSystemManager、InGameがsetupSystemsで設定する）
		system::combat::PlayerRangedAttackSystem* m_playerRangedAttackSystem{ nullptr };

		// DEBUG: デバッグ可視化・HUDの描画元（所有はInGame。リリース時に削除）
		ui::debug::DebugGizmoView* m_debugGizmoView{ nullptr };
		ui::debug::DebugHUDView* m_debugHUDView{ nullptr };
	};
} // namespace game::scene
