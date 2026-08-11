#pragma once
#include "IScene.h"
#include <vector>
#include <unordered_set>

/* core層のインクルード */
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/SystemManager.h"
#include "core/interface/ICamera.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IAnimator.h"
#include "core/interface/IEffectFactory.h"
#include "core/base/EventBus.h"
#include "core/constant/SeType.h"

/* game層のインクルード */
#include "game/factory/FactoryManager.h"
#include "game/factory/EnemySpawner.h"
#include "game/component/visual/RenderComponent.h"
#include "game/data/PlayerData.h"
#include "game/data/FileEquipmentData.h"
#include "game/event/AudioEventListener.h"
#include "game/HitStop.h"
#include "game/factory/ProjectileFactory.h"
#include "game/scene/InGameView.h"
#include <memory>

namespace core::iface
{
	class IPerformanceDataProvider; // DEBUG: 前方宣言（リリース時に削除）
} // namespace core::iface

namespace game
{
	class GameManager;  // 前方宣言
	class PauseManager; // 前方宣言

	namespace system::visual
	{
		class BattleStartSystem; // 前方宣言
	} // namespace system::visual

	namespace system::stage
	{
		class RenameTerminalSystem; // 前方宣言
	} // namespace system::stage

	namespace component::combat
	{
		struct ExtensionInventoryComponent; // 前方宣言
	} // namespace component::combat

	namespace ui::debug
	{
		class DebugGizmoView; // DEBUG: 前方宣言（リリース時に削除）
		class DebugHUDView;   // DEBUG: 前方宣言（リリース時に削除）
	} // namespace ui::debug

	namespace ui::ingame
	{
		class PlayerHUDView;     // 前方宣言
		class EquipmentSlotView; // 前方宣言
		class ObjectiveView;     // 前方宣言
		class InGameStatusView;  // 前方宣言
		class InventoryView;     // 前方宣言
		class InteractPromptView;    // 前方宣言
		class LowHealthVignetteView; // 前方宣言
		class ExtensionBoostFlashView; // 前方宣言
		class BossHUDView;           // 前方宣言
		class MiniMapView;           // 前方宣言
		class EnemyHealthBarView;    // 前方宣言
	} // namespace ui::ingame
} // namespace game

namespace game::scene
{
	/**
	 * @brief インゲームのシーンクラス
	 */
	class InGame : public IScene
	{
	public:
	  /**
	   * @brief InGameのコンストラクタ
	   * @param camera カメラのインターフェース
	   * @param renderer 描画のインターフェース
	   * @param animator アニメーションのインターフェース
	   * @param resourceManager リソース管理のインターフェース
	   * @param inputProvider 入力のインターフェース
	   * @param gameManager シーン間共有データの参照
	   * @param pauseManager ポーズ状態の参照
	   */
	  InGame(core::iface::ICamera& camera,
		  core::iface::IRenderer& renderer,
		  core::iface::IAnimator& animator,
		  core::iface::IResourceManager& resourceManager,
		  core::iface::IInputProvider& inputProvider,
		  GameManager& gameManager,
		  PauseManager& pauseManager);

	  /**
	   * @brief InGameのデストラクタ
	   * @note 前方宣言のみのメンバをunique_ptrで保持するため、実装は.cpp側で定義する
	   */
	  ~InGame() override;

	  /**
	   * @brief シーンの更新処理
	   * @param deltaTime フレーム間の時間差
	   */
	  void update(float deltaTime) override;

	  /**
	   * @brief シーンの描画処理
	   */
	  void draw() override;

	  /**
	   * @brief ポーズ状態の変化に応じてマウスカーソルの表示を切り替える
	   *
	   * 戦闘中は隠しているが、ポーズメニューはマウスでも操作できる必要がある
	   * @param isPaused ポーズ中ならtrue
	   */
	  void onPauseChanged(bool isPaused) override;

	private:
		/* コンストラクタで参照する関数 */
		void loadResources();
		void spawnEntities();
		void setupSystems();
		void setupEvents();

		/**
		 * @brief Eキーによるインベントリの開閉を処理する
		 *
		 * 開いている間は時間を止める（PauseReason::Inventory）。
		 * ポーズメニューと同時に開くと、どちらのキーが効いているのか
		 * 分からなくなるため、他の理由で止まっている間は開かない
		 */
		void updateInventory();

		/**
		 * @brief リネーム端末の前でのF2による付け替え画面の開閉を処理する
		 *
		 * どこでも開けるインベントリ（Eキー）と違い、端末の前でしか開かない。
		 * 「付け替えるためにブロックを探す」という道中の目的を作るための制限
		 */
		void updateRenameTerminal();

		/**
		 * @brief 付け替え画面でのマス選択を処理する
		 *
		 * 掴む→もう1つ押す、の2手で入れ替える。1手で入れ替えると
		 * どれと交換されたのかが分からないまま能力だけが変わる。
		 * マスの位置はViewしか知らないため、指しているマスはViewへ問い合わせる
		 */
		void updateSwapSelection();

		/**
		 * @brief 掴んでいるものと指定のマスの入れ替えを要求する
		 *
		 * クリックで置いた場合とドラッグして離した場合の両方から呼ぶ
		 * @param inventory プレイヤーの拡張子インベントリ
		 * @param targetIndex 入れ替え相手の位置（m_acquired 上の添字）
		 */
		void requestSwap(const component::combat::ExtensionInventoryComponent& inventory,
		    int targetIndex);

		/**
		 * @brief UI操作の効果音を鳴らす
		 *
		 * 掴む・置く・開閉はイベントを介さずシーンが直接受け取る操作なので、
		 * 音もここから鳴らす（入れ替えの成立音だけは結果のイベントを購読して鳴らす）
		 * @param seType 鳴らすSEの種別
		 */
		void playUiSe(core::constant::SeType seType) const;

		/**
		 * @brief インベントリの開閉をまとめて反映する
		 * @param isOpen 開くならtrue
		 * @param isSwapMode 付け替え操作を受け付ける状態で開くか
		 */
		void setInventoryOpen(bool isOpen, bool isSwapMode);

		/**
		 * @brief プレイヤーの現在のパラメータをログへ出力する
		 *
		 * 装備ファイルのボーナスが実際にパラメータへ乗っているかを、
		 * 反映の前後で見比べて確かめるために使う
		 * @param label ログの先頭に付ける見出し（"装備前" / "装備後"）
		 */
		void logPlayerParameters(const char* label) const;

		/**
		 * @brief GameManager にリザルトデータを保存する
		 * @param isVictory 勝利かどうか
		 */
		void saveResultData(bool isVictory) noexcept;

		/**
		 * @brief ボス（Mac）をステージ定義の位置に生成する
		 *
		 * 開始時ではなく、配置された雑魚を全滅させてから呼ぶ。撃破判定用にIDを保持する。
		 */
		void spawnBoss();

		/**
		 * @brief 生き残っている敵をまとめて撃破扱いにする
		 *
		 * ボスを倒した時点で決着なので、ボスが召喚した雑魚が残っていても
		 * 一緒に片付ける。以降の消失演出は通常の撃破と同じくEnemyDeathSystemが担う。
		 * @param excludedId 対象から外すEntityId（撃破済みのボス自身）
		 */
		void killRemainingEnemies(core::ecs::EntityId excludedId) noexcept;

		// 各クラスにイベントバスの参照を渡したいため先にメンバ変数として宣言しておく。
		//
		// 【重要】購読者（SystemManagerが持つ各System・m_audioEventListener）より
		// 必ず前に宣言すること。メンバの破棄は宣言の逆順で行われるため、
		// これより後に宣言すると購読者のSubscriptionが解除される時点で
		// EventBusが破棄済みになり、解放後アクセスになる
		core::base::EventBus m_eventBus;

		core::ecs::EntityManager 	m_entityManager;
		core::ecs::ComponentManager m_componentManager;
		core::ecs::SystemManager 	m_systemManager;

		core::iface::ICamera          &m_camera;
		core::iface::IRenderer        &m_renderer;
		core::iface::IAnimator        &m_animator;
		core::iface::IResourceManager &m_resourceManager;
		core::iface::IInputProvider   &m_inputProvider;
		GameManager& m_gameManager;
		PauseManager& m_pauseManager;
		data::FileEquipmentData       &m_fileEquipmentData;
		core::iface::IEffectFactory& m_effectFactory;

		game::factory::FactoryManager m_factoryManager;
		game::factory::EnemySpawner m_enemySpawner;
		game::factory::ProjectileFactory m_projectileFactory;
		game::data::PlayerData m_playerData;
		InGameView m_view;

		core::ecs::EntityId m_playerId{core::ecs::INVALID_ENTITY_ID};
		core::ecs::EntityId m_macId{ core::ecs::INVALID_ENTITY_ID };

		// 開始時に配置した雑魚のID集合。全滅（空になる）を検知してボスを出現させる。
		// ボスが召喚する雑魚は含めない（開始時のぶんだけを数える）
		std::unordered_set<core::ecs::EntityId> m_stageEnemyIds{};

		std::unique_ptr<game::event::AudioEventListener> m_audioEventListener;

		// 開始演出（READY / FIGHT!）の参照。クリアタイムの計測開始を遅らせるために読む
		// （所有はSystemManager）
		system::visual::BattleStartSystem* m_battleStartSystem{ nullptr };

		// DEBUG: ワールド空間デバッグ可視化・常時デバッグHUD（リリース時にまとめて削除）
		std::unique_ptr<ui::debug::DebugGizmoView> m_debugGizmoView;
		std::unique_ptr<ui::debug::DebugHUDView> m_debugHUDView;

		// プレイヤーステータス（左下のHP・能力値）のView
		std::unique_ptr<ui::ingame::PlayerHUDView> m_playerHUDView;

		// 装備スロット（右下）のView
		std::unique_ptr<ui::ingame::EquipmentSlotView> m_equipmentSlotView;

		// 目標表示（左上）のView
		std::unique_ptr<ui::ingame::ObjectiveView> m_objectiveView;
		std::unique_ptr<ui::ingame::InGameStatusView> m_statusView;

		// 拡張子インベントリ（Eキーで開閉）
		std::unique_ptr<ui::ingame::InventoryView> m_inventoryView;

		// 設置物への接近案内（吹き出し）
		std::unique_ptr<ui::ingame::InteractPromptView> m_interactPromptView;

		// 付け替え端末への接近判定（所有はSystemManager）。近くにいる端末をViewへ渡す
		system::stage::RenameTerminalSystem* m_renameTerminalSystem{ nullptr };

		// 付け替え操作の状態。位置は ExtensionInventoryComponent::m_acquired 上の添字で、
		// -1 は「掴んでいない」。操作を受け取るのはシーン、描くのはView、
		// 能力の差し替えはSystemと役割を分けている
		bool m_isSwapMode{ false };
		int m_swapHeldIndex{ -1 };

		// マウス左ボタンの前フレームの状態。押した瞬間だけを取り出すために持つ
		bool m_wasMouseLeftDown{ false };

		// 低HP警告のビネットのView
		std::unique_ptr<ui::ingame::LowHealthVignetteView> m_lowHealthVignetteView;
		std::unique_ptr<ui::ingame::ExtensionBoostFlashView> m_extensionBoostFlashView;

		// ボスHP（上中央）のView
		std::unique_ptr<ui::ingame::BossHUDView> m_bossHUDView;

		// ミニマップ（右上）のView
		std::unique_ptr<ui::ingame::MiniMapView> m_miniMapView;

		// 敵の頭上HPバーのView
		std::unique_ptr<ui::ingame::EnemyHealthBarView> m_enemyHealthBarView;

		// クリティカル・撃破の瞬間に時間を止める
		HitStop m_hitStop{};

		// 進行トラッキング
		float m_elapsedTime{0.0f};
		int   m_killCount{0};
		float m_totalDamageTaken{0.0f};

		// クリアタイムを計測中か。ボスを倒した瞬間にfalseになり、
		// 消失フェードや勝利遷移までの演出時間はタイムに含めない
		bool m_isTimeMeasuring{ true };

		// EventBusの購読ハンドル。このクラスが破棄されると自動で解除される
		std::vector<core::base::EventBus::Subscription> m_subscriptions{};
	};
} // namespace game::scene