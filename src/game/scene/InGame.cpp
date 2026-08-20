#include "InGame.h"

/* core層 */
#include "core/interface/ILogger.h"
#include "core/utility/Log.h"
#include "core/interface/IEffectFactory.h"
#include "core/interface/IAudioManager.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/ILighting.h"
#include "core/interface/IShadowMap.h"
#include "core/utility/Color.h"
#include "core/base/ServiceLocator.h"
#include "core/constant/SeType.h"
#include "core/data/ResultData.h"
#include "core/data/FileExtensionType.h"
#include "core/utility/MathConstants.h"
/* game層 */
#include "game/factory/FactoryInitializer.h"
#include "game/system/movement/InputSystem.h"
#include "game/system/movement/MoveSystem.h"
#include "game/system/movement/PhysicsSystem.h"
#include "game/system/movement/GroundingSystem.h"
#include "game/system/movement/FallOutSystem.h"
#include "game/system/stage/BlockBreakSystem.h"
#include "game/system/stage/BlockDebrisSystem.h"
#include "game/system/stage/ExtensionPickupSystem.h"
#include "game/system/stage/RenameTerminalSystem.h"
#include "game/system/stage/BossGateSystem.h"
#include "game/system/movement/FootstepSystem.h"
#include "game/component/movement/TransformComponent.h"
#include "game/actor/Player.h"
#include "game/GameManager.h"
#include "game/PauseManager.h"
#include "game/SettingsManager.h"
#include "game/component/visual/RenderComponent.h"
#include "game/component/combat/HealthComponent.h"
#include "game/component/visual/HitEffectComponent.h"
#include "game/system/visual/AnimationSystem.h"
#include "game/system/combat/CollisionSystem.h"
#include "game/system/visual/HitEffectSystem.h"
#include "game/system/combat/EnemyDeathSystem.h"
#include "game/system/combat/PlayerDeathSystem.h"
#include "game/system/ai/DetectionSystem.h"
#include "game/system/visual/DetectionAlertVisualsSystem.h"
#include "game/system/visual/DamagePopupSystem.h"
#include "game/system/visual/AttackTelegraphVisualsSystem.h"
#include "game/system/visual/TelegraphVisualsSystem.h"
#include "game/system/visual/EffectSystem.h"
#include "game/system/visual/LightSystem.h"
#include "game/system/visual/TextureScrollSystem.h"
#include "game/system/visual/WeaponAttachSystem.h"
#include "game/constant/PropId.h"
#include "core/interface/IEffectFactory.h"
#include "game/system/combat/AttackSystem.h"
#include "game/system/combat/ExtensionEquipSystem.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/combat/PlayerStatsComponent.h"
#include "game/component/combat/PlayerStatBaseComponent.h"
#include "game/component/combat/ExtensionInventoryComponent.h"
#include "game/constant/ModelId.h"
#include "game/constant/AnimationId.h"
#include "game/constant/ProjectileId.h"
#include "game/constant/EnemyType.h"
#include "game/component/EnemyTypeComponent.h"
#include "game/component/TagComponent.h"
#include "game/scene/SceneManager.h"
#include "game/scene/SceneType.h"
#include "game/system/ai/MeleeChaseAISystem.h"
#include "game/system/ai/RangeKeepAISystem.h"
#include "game/system/ai/EnemyRangedAttackSystem.h"
#include "game/system/ai/MacAISystem.h"
#include "game/component/ai/AIComponent.h"
#include "game/component/ai/MeleeChaseAIComponent.h"
#include "game/component/ai/RangeKeepAIComponent.h"
#include "game/component/ai/MacAIComponent.h"
#include "game/system/camera/CameraSystem.h"
#include "game/component/camera/CameraComponent.h"
#include "game/system/combat/TargetingSystem.h"
#include "game/component/combat/AimComponent.h"
#include "game/system/combat/ProjectileSystem.h"
#include "game/system/combat/ProjectileReflectSystem.h"
#include "game/system/combat/ProjectileBlockSystem.h"
#include "game/system/combat/PlayerRangedAttackSystem.h"
#include "game/system/combat/PlayerAttackComboSystem.h"
#include "game/system/visual/PlayerChargeVisualsSystem.h"
#include "game/system/visual/CriticalVisualsSystem.h"
#include "game/system/camera/ChargeZoomSystem.h"
#include "game/system/camera/DamageShakeSystem.h"
#include "game/system/visual/MacAwakenEffectSystem.h"
#include "game/system/visual/BackgroundParticleSystem.h"
#include "game/system/visual/HardAuraVisualsSystem.h"
#include "game/system/visual/RimLightVisualsSystem.h"
#include "game/system/visual/ShockwaveVisualsSystem.h"
#include "game/system/visual/HudDropSystem.h"
#include "game/system/visual/BattleStartSystem.h"
#include "game/ui/debug/DebugGizmoView.h"            // DEBUG: リリース時に削除
#include "game/ui/debug/DebugHUDView.h"              // DEBUG: リリース時に削除
#include "game/ui/ingame/PlayerHUDView.h"
#include "game/ui/ingame/EquipmentSlotView.h"
#include "game/ui/ingame/ObjectiveView.h"
#include "game/ui/ingame/InGameStatusView.h"
#include "game/ui/ingame/InventoryView.h"
#include "game/ui/ingame/InteractPromptView.h"
#include "game/ui/ingame/LowHealthVignetteView.h"
#include "game/ui/ingame/ExtensionBoostFlashView.h"
#include "game/ui/ingame/BossHUDView.h"
#include "game/ui/ingame/MiniMapView.h"
#include "game/ui/ingame/EnemyHealthBarView.h"
#include "core/interface/IPerformanceDataProvider.h" // DEBUG: リリース時に削除
#include "core/constant/DebugFlags.h"
#include "game/event/InGameEvents.h"
#include "core/utility/Probe.h" // 一時: メモリ調査用（原因特定後に削除）

/* 標準のインクルード */
#include <cassert>
#include <stdexcept>
#include <cmath>
#include <array>
#include <vector>
#include <string_view>
#include <utility>

namespace
{
	/**
	 * @brief 弾の当たり判定半径を決める
	 *
	 * projectileData.json の radius が 0 ならモデル実寸から自動計算し、0以外ならその値を使う。
	 * @param resourceManager リソース管理インターフェース
	 * @param meta 弾のメタデータ
	 * @param modelHandle 弾のモデルハンドル
	 * @return 当たり判定半径
	 */
	float resolveProjectileRadius(core::iface::IResourceManager& resourceManager,
	    const core::data::ProjectileMetadata& meta, int modelHandle)
	{
		return meta.m_radius > 0.0f
		           ? meta.m_radius
		           : resourceManager.computeBoundingRadius(modelHandle, meta.m_scale);
	}

	/** @brief Safariが投げるタブ弾の見た目一式 */
	struct TabProjectileSetup
	{
		core::data::ProjectileMetadata m_meta{};
		std::vector<game::system::ai::RangedProjectileVisual> m_visuals{};
	};

	/**
	 * @brief タブ弾（3種ランダム）のモデルと当たり判定半径を解決する
	 * @param resourceManager リソース管理インターフェース
	 * @return 解決済みのメタデータと見た目一覧
	 */
	TabProjectileSetup buildTabProjectileSetup(core::iface::IResourceManager& resourceManager)
	{
		using namespace game::constant;

		TabProjectileSetup setup{};
		setup.m_meta = resourceManager.getProjectileMetadata(projectile_id::ENEMY_SAFARI_TAB);

		constexpr std::array<std::string_view, 3> TAB_MODEL_IDS{
			model_id::TAB_STORAGE_FULL,
			model_id::TAB_SAFARI_ERROR,
			model_id::TAB_XCODE_BUILDING
		};

		setup.m_visuals.reserve(TAB_MODEL_IDS.size());
		for (const auto modelId : TAB_MODEL_IDS)
		{
			const int handle{ resourceManager.loadModelById(modelId) };
			core::probe::mark(std::format("        tab load  : {}", modelId));
			const float radius{ resolveProjectileRadius(resourceManager, setup.m_meta, handle) };
			core::probe::mark(std::format("        tab radius: {}", modelId));
			setup.m_visuals.push_back({ handle, radius });
		}
		return setup;
	}

	/** @brief ボスが投げるレインボー弾の見た目一式 */
	struct RainbowSetup
	{
		core::data::ProjectileMetadata m_meta{};
		int m_handle{ -1 };
		float m_radius{ 0.0f };
		core::Vector3 m_center{};
	};

	/**
	 * @brief レインボー弾のモデル・半径・見た目中心を解決する
	 * @param resourceManager リソース管理インターフェース
	 * @return 解決済みの設定
	 */
	RainbowSetup buildRainbowSetup(core::iface::IResourceManager& resourceManager)
	{
		RainbowSetup setup{};
		setup.m_meta = resourceManager.getProjectileMetadata(game::constant::projectile_id::MAC_RAINBOW);
		setup.m_handle = resourceManager.loadModelById(game::constant::model_id::MAC_RAINBOW_WHEEL);
		core::probe::mark("        rainbow load  ");
		setup.m_radius = resolveProjectileRadius(resourceManager, setup.m_meta, setup.m_handle);
		core::probe::mark("        rainbow radius");
		// モデル原点が見た目の中心とズレていると回転で円軌道を描くため、中心を求めて逆補正する
		setup.m_center = resourceManager.computeBoundingCenter(setup.m_handle);
		core::probe::mark("        rainbow center");
		return setup;
	}
} // namespace

namespace game::scene
{
	InGame::InGame(core::iface::ICamera& camera,
	    core::iface::IRenderer& renderer,
	    core::iface::IAnimator& animator,
	    core::iface::IResourceManager& resourceManager,
	    core::iface::IInputProvider& inputProvider,
	    GameManager& gameManager,
	    PauseManager& pauseManager,
	    SettingsManager& settingsManager)
	    : m_camera{ camera }
	    , m_renderer{ renderer }
	    , m_animator{ animator }
	    , m_resourceManager{ resourceManager }
	    , m_inputProvider{ inputProvider }
	    , m_gameManager{ gameManager }
	    , m_pauseManager{ pauseManager }
	    , m_settingsManager{ settingsManager }
	    , m_fileEquipmentData{ gameManager.getFileEquipmentData() }
	    , m_effectFactory{ *core::base::ServiceLocator::get<core::iface::IEffectFactory>() }
	    , m_factoryManager{ m_entityManager, m_componentManager, m_resourceManager }
	    , m_enemySpawner{ m_factoryManager, m_componentManager, m_resourceManager, m_eventBus,
		    gameManager.getDifficulty() }
	    , m_projectileFactory{ m_entityManager, m_componentManager }
	    // 実データは loadResources() で設定する（コライダー自動計算がモデルロード後に確定するため）
	    , m_playerData{}
	    , m_view{ m_componentManager, m_renderer,
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_effectFactory,
		    *core::base::ServiceLocator::get<core::iface::IShadowMap>() }
	{
		loadResources();
		core::probe::mark("  InGame: loadResources");
		spawnEntities();
		core::probe::mark("  InGame: spawnEntities");
		m_audioEventListener = std::make_unique<game::event::AudioEventListener>(m_eventBus, m_playerId);
		setupSystems();
		core::probe::mark("  InGame: setupSystems");
		setupEvents();
		core::probe::mark("  InGame: setupEvents");

		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio) audio->playBgm(core::constant::BgmType::InGame);

		// 背景は「真っ黒な虚無」。床だけが闇に浮かぶ世界観のため、ほぼ黒に沈める
		// （docs/design/stage_editor.md 2-3 のパレット #000000〜#0A0E14）
		constexpr int VOID_R{ 5 };
		constexpr int VOID_G{ 7 };
		constexpr int VOID_B{ 12 };
		auto* screen{ core::base::ServiceLocator::get<core::iface::IScreen>() };
		if (screen)
			screen->setBackgroundColor(VOID_R, VOID_G, VOID_B);

		// 遠くの床を背景と同じ闇へ溶かし、ステージの果てを見せずに浮遊感を出す
		constexpr float NEAR_CLIP{ 16.0f };
		constexpr float FAR_CLIP{ 20000.0f };
		constexpr float FOG_START{ 6000.0f }; // ここから徐々に闇へ
		constexpr float FOG_END{ 16000.0f };  // ここで完全に闇へ溶ける
		m_camera.setNearFar(NEAR_CLIP, FAR_CLIP);
		if (screen)
			screen->setFog(true, VOID_R, VOID_G, VOID_B, FOG_START, FOG_END);

		// ライティングを有効化して立体感を出す。環境光は青みを残して虚無の冷たさを、
		// 上からの平行光で面の向きを示す。
		//
		// 【重要】環境光と平行光の和は255を超えないこと。配置物のマテリアルは
		// amb(1.0) dif(1.0) なので、和が255を超えると最も光の当たる面が白へ飽和する。
		// 暗いテクスチャでは気付けないが、明るい面（リネーム端末）を置くと絵が消える
		// 平行光の向きは明暗と影の両方が使う。別々に書くと片方だけ直したときに
		// 「面の明るさ」と「影の伸びる向き」が食い違うため、ここで一度だけ決める
		const core::Vector3 directionalLightDirection{ -0.3f, -1.0f, 0.4f };

		auto* lighting{ core::base::ServiceLocator::get<core::iface::ILighting>() };
		if (lighting)
		{
			constexpr int AMBIENT_R{ 88 };
			constexpr int AMBIENT_G{ 94 };
			constexpr int AMBIENT_B{ 105 };
			constexpr int DIRECTIONAL_LEVEL{ 150 };
			lighting->setEnabled(true);
			lighting->setAmbient(AMBIENT_R, AMBIENT_G, AMBIENT_B);
			lighting->setDirectionalLight(directionalLightDirection,
			    DIRECTIONAL_LEVEL, DIRECTIONAL_LEVEL, DIRECTIONAL_LEVEL);
		}

		// 影を落とすシャドウマップ。範囲はプレイヤーに追従させるので、ここでは向きと精度だけ決める。
		// 解像度を上げるほど輪郭は締まるがVRAMと描画時間が増えるため、
		// 「プレイヤー周辺だけを写す」前提の 2048 に留めている
		if (auto* shadowMap{ core::base::ServiceLocator::get<core::iface::IShadowMap>() })
		{
			constexpr int SHADOW_RESOLUTION{ 1024 };

			// 小さすぎると自分の面が自分の影に入り縞状のノイズが出る。
			// 大きくすると足元の影が本体から離れて浮いて見える
			constexpr float SHADOW_ADJUST_DEPTH{ 0.001f };

			if (shadowMap->create(SHADOW_RESOLUTION))
			{
				shadowMap->setLightDirection(directionalLightDirection);
				shadowMap->setAdjustDepth(SHADOW_ADJUST_DEPTH);
			}
		}

		// 3人称マウス視点のためカーソルを非表示にする（表示の切り替えは DebugFlags.h で行う）
		m_inputProvider.setMouseCursorVisible(core::constant::SHOW_MOUSE_CURSOR_IN_GAME);

		// DEBUG: ワールド空間デバッグ可視化・常時デバッグHUD（生成するかは DebugFlags.h で切り替える）。
		// 生成しない場合は両方nullptrのままで、更新も描画も呼び出し側のnull判定で飛ばされる
		if (core::constant::ENABLE_DEBUG_VIEWS)
		{
			m_debugGizmoView = std::make_unique<ui::debug::DebugGizmoView>(m_componentManager, m_renderer);
			m_debugHUDView = std::make_unique<ui::debug::DebugHUDView>(
			    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
			    *core::base::ServiceLocator::get<core::iface::IScreen>(),
			    m_componentManager,
			    m_gameManager,
			    m_pauseManager,
			    *core::base::ServiceLocator::get<core::iface::IPerformanceDataProvider>(),
			    m_effectFactory,
			    m_renderer);
			m_view.setDebugGizmoView(m_debugGizmoView.get());
			m_view.setDebugHUDView(m_debugHUDView.get());
		}

		m_playerHUDView = std::make_unique<ui::ingame::PlayerHUDView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_componentManager,
		    m_resourceManager);
		m_view.setPlayerHUDView(m_playerHUDView.get());

		m_equipmentSlotView = std::make_unique<ui::ingame::EquipmentSlotView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_fileEquipmentData,
		    m_resourceManager,
		    m_componentManager,
		    m_playerId);
		m_view.setEquipmentSlotView(m_equipmentSlotView.get());

		m_objectiveView = std::make_unique<ui::ingame::ObjectiveView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>());
		m_view.setObjectiveView(m_objectiveView.get());

		m_statusView = std::make_unique<ui::ingame::InGameStatusView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_gameManager.getDifficulty());
		m_view.setInGameStatusView(m_statusView.get());

		m_inventoryView = std::make_unique<ui::ingame::InventoryView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_componentManager,
		    m_resourceManager,
		    m_fileEquipmentData,
		    m_inputProvider);
		m_view.setInventoryView(m_inventoryView.get());

		m_interactPromptView = std::make_unique<ui::ingame::InteractPromptView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    m_renderer,
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_componentManager,
		    m_inputProvider);
		m_view.setInteractPromptView(m_interactPromptView.get());

		m_lowHealthVignetteView = std::make_unique<ui::ingame::LowHealthVignetteView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_componentManager,
		    m_resourceManager);
		m_view.setLowHealthVignetteView(m_lowHealthVignetteView.get());

		m_extensionBoostFlashView = std::make_unique<ui::ingame::ExtensionBoostFlashView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_componentManager);
		m_view.setExtensionBoostFlashView(m_extensionBoostFlashView.get());

		m_bossHUDView = std::make_unique<ui::ingame::BossHUDView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_componentManager);
		m_view.setBossHUDView(m_bossHUDView.get());

		m_miniMapView = std::make_unique<ui::ingame::MiniMapView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_componentManager);
		m_view.setMiniMapView(m_miniMapView.get());

		m_enemyHealthBarView = std::make_unique<ui::ingame::EnemyHealthBarView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_componentManager,
		    m_renderer);
		m_view.setEnemyHealthBarView(m_enemyHealthBarView.get());
	}

	InGame::~InGame()
	{
		// カーソルを戻す最後の砦。onPauseChanged は「ポーズ中だけ出す」可逆な切り替えなので、
		// ポーズからタイトルへ戻る経路では resume() が先に走って再び隠れてしまう。
		// 抜け方（死亡・勝利・タイトルへ）ごとに書くと漏れるため、終了地点に一本化する
		m_inputProvider.setMouseCursorVisible(true);

		// シャドウマップはServiceLocator側がインゲームより長生きするため、
		// シーンを抜けるときにこちらで確保を解く
		if (auto* shadowMap{ core::base::ServiceLocator::get<core::iface::IShadowMap>() })
			shadowMap->destroy();
	}

	void InGame::onPauseChanged(bool isPaused)
	{
		// ポーズ中はメニューをマウスで操作できるように出し、再開したら戦闘用に隠す
		m_inputProvider.setMouseCursorVisible(isPaused);
	}

	void InGame::loadResources()
	{
		// 先にモデルをロードして自動計算を実行
		m_resourceManager.loadModelById(constant::model_id::PLAYER);

		// モデルロード後に再度メタデータを取得してPlayerDataを更新
		auto playerMeta{ m_resourceManager.getMetadata(constant::model_id::PLAYER) };
		if (!playerMeta.has_value())
		{
			core::log::info("ERROR: Playerのメタデータが見つかりません");
			throw std::runtime_error("Playerのメタデータの読み込みに失敗しました");
		}
		m_playerData = game::data::PlayerData::fromMetadata(playerMeta.value());
	}

	void InGame::logPlayerParameters(const char* label) const
	{
		core::log::info("Player[{}] HP={} ATK={} DEF={} SPD={} 攻撃範囲={} クールダウン={} クリティカル率={} クリティカル倍率={}",
		    label,
		    m_playerData.getMaxHp(),
		    m_playerData.getAttackPower(),
		    m_playerData.getDefence(),
		    m_playerData.getMoveSpeed(),
		    m_playerData.getAttackRange(),
		    m_playerData.getAttackCooldown(),
		    m_playerData.getCriticalRate(),
		    m_playerData.getCriticalMultiplier());
		core::log::info("Player[{}] 弾速ボーナス={} 飛距離ボーナス={}",
		    label,
		    m_playerData.getProjectileSpeedBonus(),
		    m_playerData.getProjectileRangeBonus());
	}

	void InGame::spawnEntities()
	{
		game::factory::FactoryInitializer initializer(m_factoryManager, m_resourceManager,
		    m_entityManager, m_componentManager);

		// 装備で実際にパラメータが動いたかを追えるよう、反映の前後をログに出す
		logPlayerParameters("装備前");

		// 強化前の値を控えておく。HUDが「今この能力は強化されているか」を判定する基準になる。
		// 装備ボーナスを載せたあとでは素の値を復元できないので、必ずここで取る
		component::combat::PlayerStatBaseComponent base{};
		base.m_maxHp = m_playerData.getMaxHp();
		base.m_defence = m_playerData.getDefence();
		base.m_attackPower = m_playerData.getAttackPower();
		base.m_attackRange = m_playerData.getAttackRange();
		base.m_criticalRate = m_playerData.getCriticalRate();
		base.m_moveSpeed = m_playerData.getMoveSpeed();

		// 拡張子ボーナスをPlayerDataに反映
		for (int i{ 0 }; i < data::FileEquipmentData::MAX_SLOTS; ++i)
		{
			if (m_fileEquipmentData.hasSelection(i))
			{
				m_playerData.applyExtensionBonus(
				    m_resourceManager.getExtensionBonus(m_fileEquipmentData.getExtensionType(i)));
			}
		}

		logPlayerParameters("装備後");

		// 控えた素の値も一緒に渡す。Playerのコンポーネント一式はPlayerクラスが組み立てる
		initializer.initializePlayer(m_playerData, base);
		core::probe::mark("    spawn: initializePlayer");

		m_playerId = m_factoryManager.getPlayerFactory().getPlayer().getId();

		// プレイヤー専用コンポーネント（CameraComponent、AimComponent、PlayerChargeComponent）
		// は Player.cpp のコンストラクタで初期化済

		// stageData.jsonのplayerStartを初期位置・初期向き（モデル・カメラyaw）へ反映する。
		// rotationYは度数法なのでラジアンへ変換する
		const auto& playerStart{ m_resourceManager.getStageMetadata().m_playerStart };
		auto& playerTransform{ m_componentManager.get<component::movement::TransformComponent>(m_playerId) };
		playerTransform.m_position = playerStart.m_position;

		const float startYawRad{ playerStart.m_rotationY * core::utility::DEG_TO_RAD };
		// モデルの正面は -Z 向きで、MoveSystem も進行方向に π を足した角度を入れている。
		// カメラと同じ yaw をそのまま入れるとカメラ側を向いてしまうため、ここでも π を足して背中を向ける
		playerTransform.m_rotation.y = startYawRad + core::utility::PI;
		if (m_componentManager.has<component::camera::CameraComponent>(m_playerId))
			m_componentManager.get<component::camera::CameraComponent>(m_playerId).m_yaw = startYawRad;

		// 地面は stageData.json の props[] が持つ（単一の水平地面は坂と競合するため生成しない）
		initializer.initializeProps();
		core::probe::mark("    spawn: initializeProps");

		// ステージ定義の点光源（青い道中・白銀のアリーナなどの明暗演出）
		initializer.initializeLights();
		core::probe::mark("    spawn: initializeLights");

		// 生成される敵の追跡対象をプレイヤーに設定してからスポーンする
		m_enemySpawner.setTargetEntity(core::ecs::Entity(m_playerId));
		m_enemySpawner.spawnStageEnemies();
		core::probe::mark("    spawn: spawnStageEnemies");

		// 開始時に配置された雑魚のIDを控える。ボスはまだ出さず、これらを全滅させてから出現させる。
		// この時点ではボスが未生成なので、敵種を持つEntity＝開始時の雑魚だけが集まる
		for (const auto enemyId : m_componentManager.getAllEntities<component::EnemyTypeComponent>())
			m_stageEnemyIds.insert(enemyId);

		// 雑魚が1体もいないステージ定義（テスト用など）なら、すぐボスを出す
		if (m_stageEnemyIds.empty())
			spawnBoss();
	}

	void InGame::setupSystems()
	{
		// システム登録
		// 開始演出（READY → FIGHT!）。構築時点で操作と敵AIを止めるため、
		// 入力を読むInputSystemより先に登録して解禁も同じフレーム内で先に済ませる
		m_battleStartSystem = m_systemManager.registerSystem<game::system::visual::BattleStartSystem>(
		    m_componentManager,
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_inputProvider,
		    m_playerId);
		core::probe::mark("      sys: BattleStartSystem");
		m_view.setBattleStartSystem(m_battleStartSystem);

		m_systemManager.registerSystem<game::system::movement::InputSystem>(m_componentManager, m_playerId, m_inputProvider);
		core::probe::mark("      sys: InputSystem");
		// カメラ演出（Zoom/Shake）はCameraSystemより前に走らせ、合成結果をCameraEffectComponentへ書いておく
		m_systemManager.registerSystem<game::system::camera::ChargeZoomSystem>(m_componentManager, m_playerId);
		core::probe::mark("      sys: ChargeZoomSystem");
		m_systemManager.registerSystem<game::system::camera::DamageShakeSystem>(m_componentManager, m_eventBus, m_playerId);
		core::probe::mark("      sys: DamageShakeSystem");
		// ボス覚醒演出（ズーム・シェイク・赤ビネット）。CameraSystemより前に走らせて演出チャンネルを書く。
		// 描画（赤ビネット）はInGameViewの描画フェーズから呼ぶためポインタを渡す
		auto* macAwakenEffect{ m_systemManager.registerSystem<game::system::visual::MacAwakenEffectSystem>(
			m_componentManager, m_eventBus,
			*core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
			*core::base::ServiceLocator::get<core::iface::IScreen>(),
			m_playerId) };
		core::probe::mark("      sys: MacAwakenEffectSystem");
		m_view.setMacAwakenEffectSystem(macAwakenEffect);
		// カメラはMoveSystemより前に更新し、最新のyawで移動方向を計算させる
		m_systemManager.registerSystem<game::system::camera::CameraSystem>(m_componentManager, m_playerId, m_inputProvider, m_camera, m_settingsManager.getControl());
		core::probe::mark("      sys: CameraSystem");
		m_systemManager.registerSystem<game::system::movement::MoveSystem>(m_componentManager, m_playerId, m_playerData.getDashMultiplier());
		core::probe::mark("      sys: MoveSystem");
		// 照準の敵捕捉判定（カメラ更新後・描画前に走らせる）
		m_systemManager.registerSystem<game::system::combat::TargetingSystem>(m_componentManager);
		core::probe::mark("      sys: TargetingSystem");
		// 発射入力→弾生成（生成はPhysicsSystemより前でよい）。弾定義はjsonから取得する。
		// Window弾の見た目はビルボード（板に貼ったWindow画像）で描くので、その画像を先に読む
		auto projectileMeta{ m_resourceManager.getProjectileMetadata(constant::projectile_id::PLAYER_WINDOW) };
		const int windowBillboard{ projectileMeta.m_imageId.empty() ? -1 : m_resourceManager.loadImageById(projectileMeta.m_imageId) };
		// 溜め切った弾は別のWindowロゴで描くので、そちらの画像も読んでおく
		const int chargedWindowBillboard{ projectileMeta.m_chargedImageId.empty()
			                                  ? -1
			                                  : m_resourceManager.loadImageById(projectileMeta.m_chargedImageId) };

		// 装備ファイルのボーナスを載せた弾の性能をPlayerStatsComponentへ入れる。
		// 「速さ」と「距離」は別々のボーナスとして独立に効かせたいので、寿命ではなく飛距離で持つ
		// （寿命は発射時に 飛距離÷弾速 で引き直される）
		const float baseProjectileSpeed{ projectileMeta.m_speed };
		const float baseProjectileRange{ projectileMeta.m_speed * projectileMeta.m_lifetime };

		if (auto* stats{ m_componentManager.tryGet<component::combat::PlayerStatsComponent>(m_playerId) })
		{
			stats->m_projectileSpeed = baseProjectileSpeed + m_playerData.getProjectileSpeedBonus();
			stats->m_projectileRange = baseProjectileRange + m_playerData.getProjectileRangeBonus();
		}

		// 弾の素の性能はここで初めて分かるので、控えの残りを埋める
		if (auto* base{ m_componentManager.tryGet<component::combat::PlayerStatBaseComponent>(m_playerId) })
		{
			base->m_projectileSpeed = baseProjectileSpeed;
			base->m_projectileRange = baseProjectileRange;
		}

		auto* rangedAttack{ m_systemManager.registerSystem<game::system::combat::PlayerRangedAttackSystem>(
			m_componentManager, m_playerId, m_projectileFactory, projectileMeta, windowBillboard, chargedWindowBillboard) };
		core::probe::mark("      sys: PlayerRangedAttackSystem");
		// レティクルがクールダウンの残量を読むため、Viewへ参照を渡す
		m_view.setPlayerRangedAttackSystem(rangedAttack);
		m_systemManager.registerSystem<game::system::movement::PhysicsSystem>(m_componentManager, m_gameManager, m_playerData.getJumpForce(), m_playerData.getGravity(), m_playerData.getMaxFallSpeed());
		core::probe::mark("      sys: PhysicsSystem");
		// 弾の寿命・再アーム・破棄（当たり判定するAttackSystemより前で再アームする）
		m_systemManager.registerSystem<game::system::combat::ProjectileSystem>(m_componentManager, m_entityManager, m_eventBus);
		core::probe::mark("      sys: ProjectileSystem");
		// 敵弾をプレイヤーのWindow弾で跳ね返す（移動後・ダメージ判定AttackSystemより前に判定する）
		m_systemManager.registerSystem<game::system::combat::ProjectileReflectSystem>(m_componentManager);
		core::probe::mark("      sys: ProjectileReflectSystem");
		// 壁・ブロックにぶつかった弾を消す。壁越しに当たらないよう、ダメージ判定より前に消す
		m_systemManager.registerSystem<game::system::combat::ProjectileBlockSystem>(m_componentManager, m_entityManager);
		core::probe::mark("      sys: ProjectileBlockSystem");

		m_systemManager.registerSystem<game::system::visual::AnimationSystem>(m_componentManager, m_animator, m_eventBus);
		core::probe::mark("      sys: AnimationSystem");

		// ボス出現で入り口を塞ぐ扉。押し返しの前に動かして、その位置で当たり判定させる
		m_systemManager.registerSystem<game::system::stage::BossGateSystem>(m_componentManager, m_eventBus);
		core::probe::mark("      sys: BossGateSystem");

		m_systemManager.registerSystem<game::system::combat::CollisionSystem>(m_componentManager);
		core::probe::mark("      sys: CollisionSystem");
		// 障害物の押し返し後に、床・坂の傾いた面へ足を乗せる（坂はAABBで表せないため専用処理）
		m_systemManager.registerSystem<game::system::movement::GroundingSystem>(m_componentManager);
		core::probe::mark("      sys: GroundingSystem");
		// 奈落へ落ちた者の始末。接地が終わって位置と足場が確定してから判定する
		m_systemManager.registerSystem<game::system::movement::FallOutSystem>(m_componentManager, m_eventBus);
		core::probe::mark("      sys: FallOutSystem");
		// 足音は「進んだ距離」で数えるため、押し返しと接地が終わって位置が確定してから走らせる
		m_systemManager.registerSystem<game::system::movement::FootstepSystem>(m_componentManager, m_playerId);
		core::probe::mark("      sys: FootstepSystem");
		// AI行動分割：近接追跡型敵を駆動
		m_systemManager.registerSystem<game::system::ai::MeleeChaseAISystem>(m_componentManager);
		core::probe::mark("      sys: MeleeChaseAISystem");
		// AI行動分割：遠距離維持型敵を駆動
		m_systemManager.registerSystem<game::system::ai::RangeKeepAISystem>(m_componentManager);
		core::probe::mark("      sys: RangeKeepAISystem");
		// 遠距離維持型敵の弾発射（Safariのタブ投擲）。見た目は3種のタブモデルからランダムに選ぶ
		auto tabSetup{ buildTabProjectileSetup(m_resourceManager) };
		m_systemManager.registerSystem<game::system::ai::EnemyRangedAttackSystem>(
		    m_componentManager, m_projectileFactory, tabSetup.m_meta, std::move(tabSetup.m_visuals));
		core::probe::mark("      sys: EnemyRangedAttackSystem");

		// ボス（Mac）のFSM駆動。遠距離はレインボー弾を扇状に、召喚はEnemySpawner経由で行う
		const auto rainbow{ buildRainbowSetup(m_resourceManager) };
		m_systemManager.registerSystem<game::system::ai::MacAISystem>(
		    m_componentManager, m_eventBus, m_projectileFactory, m_enemySpawner,
		    rainbow.m_meta, rainbow.m_handle, rainbow.m_radius, rainbow.m_center);
		core::probe::mark("      sys: MacAISystem");

		// 敵がプレイヤーを発見した瞬間を検知（全敵共通）。発見演出のトリガーになる
		m_systemManager.registerSystem<game::system::ai::DetectionSystem>(m_componentManager, m_eventBus);
		core::probe::mark("      sys: DetectionSystem");

		// プレイヤーの近接攻撃入力をコンボの段数へ振り分ける（攻撃の成立はAttackSystem）
		m_systemManager.registerSystem<game::system::combat::PlayerAttackComboSystem>(
		    m_componentManager, m_playerId);
		core::probe::mark("      sys: PlayerAttackComboSystem");

		m_systemManager.registerSystem<game::system::combat::AttackSystem>(
		    m_componentManager, m_eventBus);
		core::probe::mark("      sys: AttackSystem");
		// 壊せるブロックを近接攻撃で削る。打撃回数で壊れるためAttackSystemの
		// ダメージ計算には乗せず、攻撃が成立したフレームだけを見る。
		// m_justFired はAttackSystemが毎フレーム立て直すので必ずその後に置く
		m_systemManager.registerSystem<game::system::stage::BlockBreakSystem>(
		    m_componentManager, m_entityManager, m_renderer, m_resourceManager, m_eventBus,
		    m_enemySpawner, m_playerId);
		core::probe::mark("      sys: BlockBreakSystem");

		// 壊れたブロックの破片を飛散させる。壊れた直後から動かしたいので破壊の直後に置く
		m_systemManager.registerSystem<game::system::stage::BlockDebrisSystem>(
		    m_componentManager, m_entityManager, m_renderer);
		core::probe::mark("      sys: BlockDebrisSystem");

		// 落ちた欠片の落下・浮遊・取得。破壊の直後に生成されるので破片の次に置く
		m_systemManager.registerSystem<game::system::stage::ExtensionPickupSystem>(
		    m_componentManager, m_entityManager, m_eventBus, m_playerId);
		core::probe::mark("      sys: ExtensionPickupSystem");

		// 拡張子の付け替え端末への接近判定。案内の表示とF2の受付がこの結果を見る
		m_renameTerminalSystem = m_systemManager.registerSystem<game::system::stage::RenameTerminalSystem>(
		    m_componentManager, m_playerId);
		core::probe::mark("      sys: RenameTerminalSystem");

		// 拾った拡張子をプレイヤーの能力へ乗せる。取得の直後に反映したいので取得の次に置く
		m_systemManager.registerSystem<game::system::combat::ExtensionEquipSystem>(
		    m_componentManager, m_eventBus, m_resourceManager, m_fileEquipmentData, m_playerId);
		core::probe::mark("      sys: ExtensionEquipSystem");

		m_systemManager.registerSystem<game::system::visual::HitEffectSystem>(m_componentManager, m_eventBus);
		core::probe::mark("      sys: HitEffectSystem");
		// 死亡した敵の後始末（赤化＋ディゾルブ演出→Entity破棄＋モデルハンドルのプール返却）
		m_systemManager.registerSystem<game::system::combat::EnemyDeathSystem>(m_componentManager, m_entityManager, m_eventBus, m_enemySpawner, m_renderer);
		core::probe::mark("      sys: EnemyDeathSystem");

		// プレイヤーの死亡演出（死亡アニメ→暗転）。完了時にシーン遷移用のイベントを発行する。
		// 描画（暗転）はInGameViewの描画フェーズから呼ぶためポインタを渡す
		auto* playerDeath{ m_systemManager.registerSystem<game::system::combat::PlayerDeathSystem>(
			m_componentManager, m_eventBus,
			*core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
			*core::base::ServiceLocator::get<core::iface::IScreen>(),
			m_playerId) };
		core::probe::mark("      sys: PlayerDeathSystem");
		m_view.setPlayerDeathSystem(playerDeath);

		m_systemManager.registerSystem<game::system::visual::EffectSystem>(m_componentManager, m_eventBus, m_effectFactory);
		core::probe::mark("      sys: EffectSystem");

		// 壁などの模様を流す（貼り方をずらすだけなので描画状態に影響しない）
		m_systemManager.registerSystem<game::system::visual::TextureScrollSystem>(m_componentManager);
		core::probe::mark("      sys: TextureScrollSystem");

		// 装着武器の装着先ボーンを解決する（解決はEntityごとに一度きり。描画はInGameView）
		m_systemManager.registerSystem<game::system::visual::WeaponAttachSystem>(m_componentManager, m_renderer);
		core::probe::mark("      sys: WeaponAttachSystem");

		// LightComponentを持つエンティティの点光源を生成・追従させる（プレイヤーの携行灯など）
		if (auto* lighting{ core::base::ServiceLocator::get<core::iface::ILighting>() })
			m_systemManager.registerSystem<game::system::visual::LightSystem>(m_componentManager, *lighting);
		core::probe::mark("      sys: LightSystem");

		// プレイヤーの溜め攻撃の画面演出（集中線）。描画内容はSystemが持ち、
		// InGameViewには描画フェーズで呼び出させるためにポインタを渡す
		auto* chargeVisuals{ m_systemManager.registerSystem<game::system::visual::PlayerChargeVisualsSystem>(
			m_componentManager,
			*core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
			*core::base::ServiceLocator::get<core::iface::IScreen>(),
			m_playerId) };
		core::probe::mark("      sys: PlayerChargeVisualsSystem");
		m_view.setPlayerChargeVisualsSystem(chargeVisuals);

		// クリティカルの瞬間に弾ける集中線
		auto* criticalVisuals{ m_systemManager.registerSystem<game::system::visual::CriticalVisualsSystem>(
			m_componentManager,
			m_eventBus,
			*core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
			*core::base::ServiceLocator::get<core::iface::IScreen>(),
			m_playerId) };
		core::probe::mark("      sys: CriticalVisualsSystem");
		m_view.setCriticalVisualsSystem(criticalVisuals);

		// 虚空を流れるデータの光跡（背景の奥行きと動きを作る）
		auto* backgroundParticles{ m_systemManager.registerSystem<game::system::visual::BackgroundParticleSystem>(
			m_componentManager, m_playerId, m_renderer, m_resourceManager) };
		core::probe::mark("      sys: BackgroundParticleSystem");
		m_view.setBackgroundParticleSystem(backgroundParticles);

		// Hardの敵を包む赤いオーラ（強化されていることを戦闘中に伝える）
		auto* hardAura{ m_systemManager.registerSystem<game::system::visual::HardAuraVisualsSystem>(
			m_componentManager, m_renderer, m_resourceManager,
			m_gameManager.getDifficulty() == core::data::Difficulty::Hard) };
		core::probe::mark("      sys: HardAuraVisualsSystem");
		m_view.setHardAuraVisualsSystem(hardAura);

		// 輪郭光。RimLightComponentを付けたEntityだけが対象
		auto* rimLight{ m_systemManager.registerSystem<game::system::visual::RimLightVisualsSystem>(
			m_componentManager, m_renderer) };
		core::probe::mark("      sys: RimLightVisualsSystem");
		m_view.setRimLightVisualsSystem(rimLight);

		// 地面を走る衝撃波。ShockwaveComponentを付けたEntityが対象
		auto* shockwave{ m_systemManager.registerSystem<game::system::visual::ShockwaveVisualsSystem>(
			m_componentManager, m_renderer) };
		core::probe::mark("      sys: ShockwaveVisualsSystem");
		m_view.setShockwaveVisualsSystem(shockwave);

		// ボス覚醒でHUDを震わせて落とす
		auto* hudDrop{ m_systemManager.registerSystem<game::system::visual::HudDropSystem>(
			m_eventBus, *core::base::ServiceLocator::get<core::iface::IScreen>()) };
		core::probe::mark("      sys: HudDropSystem");
		m_view.setHudDropSystem(hudDrop);

		// 敵の発見演出（頭上の通知バッジ）。描画内容はSystemが持ち、Viewが描画フェーズで呼ぶ
		auto* detectionAlert{ m_systemManager.registerSystem<game::system::visual::DetectionAlertVisualsSystem>(
			m_componentManager,
			m_eventBus,
			m_renderer,
			*core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
			*core::base::ServiceLocator::get<core::iface::IScreen>(),
			m_resourceManager) };
		core::probe::mark("      sys: DetectionAlertVisualsSystem");
		m_view.setDetectionAlertVisualsSystem(detectionAlert);

		// 敵に与えたダメージ量の表示。描画内容はSystemが持ち、Viewが描画フェーズで呼ぶ
		auto* damagePopup{ m_systemManager.registerSystem<game::system::visual::DamagePopupSystem>(
			m_componentManager,
			m_eventBus,
			m_renderer,
			*core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
			*core::base::ServiceLocator::get<core::iface::IScreen>()) };
		core::probe::mark("      sys: DamagePopupSystem");
		m_view.setDamagePopupSystem(damagePopup);

		// 攻撃予兆（地面の攻撃範囲サークル）。描画内容はSystemが持ち、Viewが3D描画フェーズで呼ぶ
		auto* attackTelegraph{ m_systemManager.registerSystem<game::system::visual::AttackTelegraphVisualsSystem>(
			m_componentManager, m_renderer) };
		core::probe::mark("      sys: AttackTelegraphVisualsSystem");
		m_view.setAttackTelegraphVisualsSystem(attackTelegraph);

		// 汎用の攻撃予兆（TelegraphComponent駆動：円・扇）。ボスの溜め攻撃などが使う
		auto* telegraph{ m_systemManager.registerSystem<game::system::visual::TelegraphVisualsSystem>(
			m_componentManager, m_renderer) };
		core::probe::mark("      sys: TelegraphVisualsSystem");
		m_view.setTelegraphVisualsSystem(telegraph);
	}

	void InGame::setupEvents()
	{
		// Hitイベントの購読
		m_subscriptions.push_back(m_eventBus.subscribe<event::AttackHitEvent>([this](const event::AttackHitEvent& e)
		    {
				// 被ダメージ追跡（プレイヤーが攻撃を受けた場合）
				if (e.m_targetId == m_playerId)
					m_totalDamageTaken += e.m_damage;

			    // クリティカルの瞬間に一拍止めて手応えを作る。
			    // 与えたときだけで、被弾側では止めない（操作不能時間は理不尽に感じるため）
			    if (e.m_isCritical && e.m_targetId != m_playerId)
					m_hitStop.requestOnCritical(); }));
		// プレイヤー死亡演出の完了イベントの購読。
		// HPが尽きた瞬間（PlayerDeadEvent）ではなく、死亡アニメと暗転を見せ終えてから遷移する。
		// 演出中もモデルは表示し続ける（非表示にすると死亡アニメが見えなくなる）
		m_subscriptions.push_back(m_eventBus.subscribe<event::PlayerDeathSequenceFinishedEvent>(
		    [this](const event::PlayerDeathSequenceFinishedEvent&)
		    {
			    saveResultData(false);
			    auto* sceneManager{ core::base::ServiceLocator::get<game::scene::SceneManager>() };
			    sceneManager->changeScene(game::scene::SceneType::Result);
		    }));

		// 道中で湧いた雑魚も討伐対象に加える（ギャンブルボックスの外れなど）。
		m_subscriptions.push_back(m_eventBus.subscribe<event::EnemySpawnedEvent>(
		    [this](const event::EnemySpawnedEvent& e)
		    {
			    // ボス自身は雑魚ではないので入れない。m_macId への代入はこのイベントより
			    // 後（spawnが返ってから）なので、IDでは判定できず敵種で見る必要がある
			    const auto* enemyType{ m_componentManager.tryGet<component::EnemyTypeComponent>(e.m_entityId) };
			    if (enemyType != nullptr && enemyType->m_type == constant::EnemyType::Mac)
				    return;

			    if (m_macId == core::ecs::INVALID_ENTITY_ID)
				    m_stageEnemyIds.insert(e.m_entityId);
		    }));

		// 敵の死亡イベントの購読
		m_subscriptions.push_back(m_eventBus.subscribe<event::EnemyDeadEvent>([this](const event::EnemyDeadEvent& e)
		    {
			    // 倒した敵の種類をログに出す（動作確認用）。
			    // 敵種はスポーン時に確定した EnemyTypeComponent を唯一の情報源にする
			    std::string_view enemyTypeName{ "Unknown" };
			    if (const auto* type{ m_componentManager.tryGet<component::EnemyTypeComponent>(e.m_entityId) })
				    enemyTypeName = constant::toEnemyTypeName(type->m_type);
			    core::log::info("敵を撃破: {} (EntityId={})", enemyTypeName, e.m_entityId);

			    // モデルはここで非表示にしない。EnemyDeathSystemが赤化＋ディゾルブ演出を
			    // 進めながら表示し続け、演出完了時にEntityごと破棄する

			    // AIを停止する。これをしないと死亡後も移動や弾発射が続き、
			    // 「まだSafariがタブをPlayerに投げてくる」状態になる
			    if (m_componentManager.has<component::ai::AIComponent>(e.m_entityId))
				    m_componentManager.get<component::ai::AIComponent>(e.m_entityId).m_isActive = false;

			    m_killCount++;

			    // 開始時の雑魚を全滅させたらボスを出現させる。
			    // 集合に無いID（ボスの召喚した雑魚・ボス自身）はここでは無視される
			    if (m_stageEnemyIds.erase(e.m_entityId) > 0 &&
			        m_stageEnemyIds.empty() && m_macId == core::ecs::INVALID_ENTITY_ID)
				    spawnBoss();

			    // ボスを倒した瞬間に決着なので、ここでクリアタイムを止める。
			    // 消失フェードと勝利遷移は演出の時間で、プレイヤーの速さとは関係ない
			    if (e.m_entityId == m_macId)
			    {
				    m_isTimeMeasuring = false;
				    killRemainingEnemies(e.m_entityId);
			    }

			    // 勝利遷移はここ（HP0の瞬間）では行わない。ボスの死亡アニメと消失フェードを
			    // 見せ終えてから遷移したいので、EnemyVanishedEvent（消滅完了）を待つ
		    }));

		// ボスが死亡演出を終えて完全に消滅したら勝利リザルトへ遷移する。
		// プレイヤー死亡（PlayerDeathSequenceFinishedEvent）と同じく、演出を見せ終えてから切り替える
		m_subscriptions.push_back(m_eventBus.subscribe<event::EnemyVanishedEvent>(
		    [this](const event::EnemyVanishedEvent& e)
		    {
			    // ボス（Mac）が消滅したときだけ勝利遷移する
			    if (e.m_type != constant::EnemyType::Mac)
				    return;
			    saveResultData(true);
			    auto* sceneManager{ core::base::ServiceLocator::get<game::scene::SceneManager>() };
			    sceneManager->changeScene(game::scene::SceneType::Result);
		    }));
	}

	void InGame::killRemainingEnemies(core::ecs::EntityId excludedId) noexcept
	{
		// 先に対象を控えてから倒す。撃破するとEnemyDeathSystemがDeathComponentを足すので、
		// 走査しながら倒すと反復中にComponentManagerの中身が変わってしまう
		std::vector<core::ecs::EntityId> targets{};
		for (const auto entityId : m_componentManager.getAllEntities<component::combat::HealthComponent>())
		{
			if (entityId == excludedId)
				continue;

			const auto* tag{ m_componentManager.tryGet<component::TagComponent>(entityId) };
			if (tag == nullptr || tag->m_tag != constant::Tag::Enemy)
				continue;

			if (m_componentManager.get<component::combat::HealthComponent>(entityId).m_isDead)
				continue;

			targets.push_back(entityId);
		}

		// 倒し方は落下死（FallOutSystem）と揃える。HPを0にして死亡フラグを立て、
		// EnemyDeadEventを出せば、撃破数の集計も消失演出も通常どおり流れる
		for (const auto entityId : targets)
		{
			auto& health{ m_componentManager.get<component::combat::HealthComponent>(entityId) };
			health.m_currentHp = 0.0f;
			health.m_isDead = true;
			m_eventBus.publish(event::EnemyDeadEvent{ entityId });
		}
	}

	void InGame::spawnBoss()
	{
		const auto& macSpawn{ m_resourceManager.getStageMetadata().m_mac };
		if (macSpawn.m_type.empty())
			return; // ボス未定義のステージなら何もしない（勝利条件が成立しなくなる点は許容）

		// 出現シネマの間はAIが止まっており向きを追従しない。登場の瞬間に背中を見せないよう、
		// 湧いた時点でプレイヤーの方を向かせる。プレイヤーの進入方向に依らず正しくなるので、
		// ステージデータの向き（m_rotationY）はボスに限り使わない
		float spawnYawDegrees{ macSpawn.m_rotationY };
		if (const auto* playerTransform{ m_componentManager.tryGet<component::movement::TransformComponent>(m_playerId) })
		{
			const float toPlayerX{ playerTransform->m_position.x - macSpawn.m_position.x };
			const float toPlayerZ{ playerTransform->m_position.z - macSpawn.m_position.z };
			if (toPlayerX != 0.0f || toPlayerZ != 0.0f)
				spawnYawDegrees = std::atan2f(-toPlayerX, -toPlayerZ) * core::utility::RAD_TO_DEG;
		}

		core::probe::mark("  ボス出現: spawn 前");
		m_macId = m_enemySpawner.spawn(constant::toEnemyType(macSpawn.m_type), macSpawn.m_position,
		    spawnYawDegrees);
		core::probe::mark("  ボス出現: spawn 後");
		core::log::info("雑魚を全滅：ボスが出現しました (EntityId={})", m_macId);

		// BGMをボス戦へ切り替える。出現シネマと同時に変えることで、カメラが寄る瞬間に
		// 曲も切り替わり「ここからボス戦」が音でも分かる
		if (auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() })
			audio->playBgm(core::constant::BgmType::Boss);

		// 出現シネマ（カメラをボスへ寄せてシェイク→プレイヤーへ戻す）を起動する。
		// 実際の演出はMacAwakenEffectSystemが担う
		m_eventBus.publish(event::BossAppearedEvent{ m_macId });
	}

	void InGame::update(float deltaTime)
	{
		// DEBUG: 更新レート（UPS）を数える。FPS計測は描画回数を数える必要があるためdraw側で行う
		// （リリース時に削除）
		if (m_debugHUDView)
			m_debugHUDView->countGameUpdate();

		// ヒットストップ中はSystemへ渡す時間に倍率を掛ける（0なら何も進まない）。
		// 経過時間の計測もここへ揃える。止まっている間もタイマーだけ進むと、
		// 画面が止まっているのに右上の秒数だけ動いて不自然になる
		updateInventory();
		updateRenameTerminal();

		// インベントリを開いている間は時間を止める。読む画面なので、
		// 読んでいる最中に殴られるのはプレイヤーの落ち度ではなく設計の落ち度になる。
		// ただし付け替えの操作だけは止まっている間に受け付ける
		if (m_pauseManager.isPausedBy(PauseReason::Inventory))
		{
			updateSwapSelection();
			return;
		}

		const float scaledDeltaTime{ m_hitStop.apply(deltaTime) };

		// 開始演出（READY）の間はまだ動けないので、クリアタイムの計測も始めない。
		// ボス撃破後（m_isTimeMeasuring=false）も、そこから先は演出の時間なので進めない
		if (m_isTimeMeasuring &&
		    (m_battleStartSystem == nullptr || !m_battleStartSystem->isPreparing()))
			m_elapsedTime += scaledDeltaTime;
		m_systemManager.update(scaledDeltaTime);

		// 近くにいる端末をViewへ渡す。判定はSystem、表示はViewと分けているので、
		// 案内の見た目を変えても判定側を触らずに済む
		if (m_renameTerminalSystem)
			m_view.setInteractTarget(m_renameTerminalSystem->getNearTerminalId());
	}

	void InGame::updateInventory()
	{
		// 開いたキーが何であれEscで閉じられるようにする。「とりあえずEscで戻れる」は
		// どの画面でも共通の期待なので、ここだけ効かないと閉じ方を探すことになる。
		// ポーズメニュー側（Application）はインベントリで止まっている間はEscを見ないので、
		// ここで閉じてもメニューが続けて開くことはない
		if (m_pauseManager.isPausedBy(PauseReason::Inventory) &&
		    (m_inputProvider.consumeKeyPress(core::input::KeyCode::Escape) ||
		        m_inputProvider.consumePadPress(core::input::GamePadCode::ButtonCircle)))
		{
			setInventoryOpen(false, false);
			return;
		}

		// パッドは△。キーと同じく、開けたボタンでそのまま閉じられる
		if (!m_inputProvider.consumeKeyPress(core::input::KeyCode::E) &&
		    !m_inputProvider.consumePadPress(core::input::GamePadCode::ButtonTriangle))
			return;

		// 別の理由（ポーズメニュー）で止まっている間は開かない。
		// 2つの画面が重なると、どちらのキーが効いているのか分からなくなる
		if (m_pauseManager.isPausedBy(PauseReason::Inventory))
		{
			setInventoryOpen(false, false);
			return;
		}

		if (m_pauseManager.isPaused())
			return;

		// 端末の前で開いたときは、そのまま付け替えられる状態にする。
		// ここで「見るだけ」を開いてしまうと、目の前に端末があるのに
		// 一度閉じて□を押し直すことになり、付け替えられること自体に気付けない
		setInventoryOpen(true, isNearRenameTerminal());
	}

	bool InGame::isNearRenameTerminal() const
	{
		return m_renameTerminalSystem != nullptr &&
		       m_renameTerminalSystem->getNearTerminalId() != core::ecs::INVALID_ENTITY_ID;
	}

	void InGame::updateRenameTerminal()
	{
		// パッドは□
		if (!m_inputProvider.consumeKeyPress(core::input::KeyCode::F2) &&
		    !m_inputProvider.consumePadPress(core::input::GamePadCode::ButtonSquare))
			return;

		// 開いている間はF2でも閉じられる。開いたキーで閉じられないと、
		// 閉じ方を探すことになる
		if (m_pauseManager.isPausedBy(PauseReason::Inventory))
		{
			setInventoryOpen(false, false);
			return;
		}
		if (m_pauseManager.isPaused())
			return;

		// 端末の前でのみ開く。どこでも付け替えられるなら、端末を探す理由が無くなる
		if (!isNearRenameTerminal())
			return;

		setInventoryOpen(true, true);
	}

	void InGame::setInventoryOpen(bool isOpen, bool isSwapMode)
	{
		if (isOpen)
			m_pauseManager.pause(PauseReason::Inventory);
		else
			m_pauseManager.resume();

		m_isSwapMode = isOpen && isSwapMode;
		m_swapHeldIndex = -1;

		// 開閉は場面が切り替わる合図。時間が止まる／動き出すことを音でも示す
		playUiSe(isOpen ? core::constant::SeType::InventoryOpen : core::constant::SeType::UiClose);

		m_view.setInventoryOpen(isOpen);
		if (m_inventoryView)
		{
			m_inventoryView->setSwapMode(m_isSwapMode);
			m_inventoryView->setSelection(-1, -1);
			m_inventoryView->resetStatChanges();
		}

		// 開いている間はカーソルを出す。隠したままだとマウスを中央へ戻す処理
		// （getMouseDelta）が止まり、カーソルが端まで流れていく。
		// その状態で閉じると溜まったぶんが一度に効いてカメラが飛ぶ
		m_inputProvider.setMouseCursorVisible(isOpen);
	}

	void InGame::playUiSe(core::constant::SeType seType) const
	{
		if (auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() })
			audio->playSe(seType);
	}

	void InGame::requestSwap(
	    const component::combat::ExtensionInventoryComponent& inventory, int targetIndex)
	{
		// 同じ区分どうしなら並び替えになる。能力は変わらないが、
		// 見やすく並べたいという操作は通す（音だけ入れ替えと分ける）
		if (inventory.isEquipped(m_swapHeldIndex) == inventory.isEquipped(targetIndex))
			playUiSe(core::constant::SeType::ExtensionDrop);

		m_eventBus.publish(event::ExtensionSwapRequestedEvent{ m_swapHeldIndex, targetIndex });
		m_swapHeldIndex = -1;
	}

	void InGame::updateSwapSelection()
	{
		if (!m_isSwapMode)
			return;

		const auto* inventory{
			m_componentManager.tryGet<component::combat::ExtensionInventoryComponent>(m_playerId)
		};
		if (inventory == nullptr || inventory->m_acquired.empty())
			return;

		if (m_inventoryView == nullptr)
			return;

		int mouseX{ 0 };
		int mouseY{ 0 };
		m_inputProvider.getMousePosition(mouseX, mouseY);
		const int hoveredIndex{ m_inventoryView->findSlotIndexAt(mouseX, mouseY) };

		// 押した瞬間と離した瞬間を取り出す。押しっぱなしを毎フレーム見ると、
		// 1回のクリックの間に掴むと離すを何度も繰り返してしまう
		const bool isDown{ m_inputProvider.isMouseLeftPressed() };
		const bool isPressed{ isDown && !m_wasMouseLeftDown };
		const bool isReleased{ !isDown && m_wasMouseLeftDown };
		m_wasMouseLeftDown = isDown;

		// 動かせない枠へ落とそうとしたら弾く。掴んだままにしておくのは、
		// 拒否された操作で持ち物の状態まで変わるとやり直しが面倒になるため
		const bool isOverLocked{ m_inventoryView->isLockedSlotAt(mouseX, mouseY) };
		if ((isPressed || isReleased) && isOverLocked && m_swapHeldIndex >= 0)
		{
			m_inventoryView->startRejectShake(
			    ui::ingame::InventoryView::ShakeTarget::Locked);
			playUiSe(core::constant::SeType::ExtensionRejected);
			m_inventoryView->setSelection(hoveredIndex, m_swapHeldIndex);
			m_inventoryView->setDragging(isDown && m_swapHeldIndex >= 0, mouseX, mouseY);
			return;
		}

		if (isPressed)
		{
			if (hoveredIndex < 0)
			{
				// マスの外を押したら掴んでいたものを置く。取り消せないと、
				// 間違えて掴んだときに意図しない入れ替えを強いられる
				if (m_swapHeldIndex >= 0)
					playUiSe(core::constant::SeType::ExtensionDrop);
				m_swapHeldIndex = -1;
			}
			else if (m_swapHeldIndex < 0 || m_swapHeldIndex == hoveredIndex)
			{
				// 何も掴んでいなければ掴む。同じマスをもう一度押したら離す
				const bool isReleasing{ m_swapHeldIndex == hoveredIndex };
				playUiSe(isReleasing ? core::constant::SeType::ExtensionDrop
				                     : core::constant::SeType::ExtensionGrab);
				m_swapHeldIndex = isReleasing ? -1 : hoveredIndex;
			}
			else
			{
				// 掴んだまま別のマスを押した場合はその場で入れ替える
				requestSwap(*inventory, hoveredIndex);
			}
		}
		else if (isReleased && m_swapHeldIndex >= 0 && hoveredIndex >= 0 &&
		         hoveredIndex != m_swapHeldIndex)
		{
			// 掴んだまま別のマスへ運んで離した（ドラッグ＆ドロップ）。
			// 掴む・置くの2クリックと、運んで離すの1動作の両方を受けることで、
			// どちらのつもりで触っても同じ結果になる
			requestSwap(*inventory, hoveredIndex);
		}

		// ボタンを押している間だけ運んでいる扱いにする。離したあとも掴んだままなら、
		// 2クリックで置く操作の途中とみなす
		m_inventoryView->setDragging(isDown && m_swapHeldIndex >= 0, mouseX, mouseY);
		m_inventoryView->setSelection(hoveredIndex, m_swapHeldIndex);
	}

	void InGame::draw()
	{
		// DEBUG: FPS計測とCPU/メモリ取得。シーンビュー凍結中もdrawは呼ばれるため計測は続く
		// （リリース時に削除）
		if (m_debugHUDView)
			m_debugHUDView->updateOnRenderFrame();

		// 描画は InGameView へ委譲する。ボスが召喚する雑魚も実行時に増えるため、
		// スポーン時のスナップショットではなく EnemyFactory が持つ最新の敵一覧を渡す
		// 残り雑魚はボス出現条件そのものなので、その集合の生き残り数を渡す。
		// 道中で湧いたぶんも含み、ボスが召喚する雑魚は含めない（setupEvents参照）
		m_view.draw(m_playerId, static_cast<int>(m_stageEnemyIds.size()), m_macId, m_elapsedTime);
	}

	void InGame::saveResultData	(bool isVictory) noexcept
	{
		core::data::ResultData result{};
		result.m_isVictory        = isVictory;
		result.m_difficulty = m_gameManager.getDifficulty();
		result.m_elapsedTime      = m_elapsedTime;
		result.m_killCount        = m_killCount;
		result.m_totalDamageTaken = m_totalDamageTaken;

		for (int i{0}; i < data::FileEquipmentData::MAX_SLOTS; ++i)
		{
			if (m_fileEquipmentData.hasSelection(i))
				result.m_usedFiles.push_back(m_fileEquipmentData.getFilePath(i));
		}

		// 道中で拾ったぶん。持ち込みと分けて持つことで、リザルトで
		// 「何を持ち込んで、何を拾って強くなったか」を並べて見せられる
		if (const auto* inventory{
		        m_componentManager.tryGet<component::combat::ExtensionInventoryComponent>(m_playerId) })
		{
			for (const auto type : inventory->m_acquired)
				result.m_acquiredExtensions.emplace_back(core::data::toExtensionName(type));
			result.m_equippedExtensionCount = inventory->equippedCount();
		}

		m_gameManager.setResultData(result);
	}
} // namespace game::scene