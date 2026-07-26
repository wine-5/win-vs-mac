#include "InGame.h"

/* core層 */
#include "core/interface/ILogger.h"
#include "core/utility/Log.h"
#include "core/interface/IEffectFactory.h"
#include "core/interface/IAudioManager.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/ILighting.h"
#include "core/utility/Color.h"
#include "core/base/ServiceLocator.h"
#include "core/constant/SeType.h"
#include "core/data/ResultData.h"
#include "core/utility/MathConstants.h"
/* game層 */
#include "game/factory/FactoryInitializer.h"
#include "game/system/movement/InputSystem.h"
#include "game/system/movement/MoveSystem.h"
#include "game/system/movement/PhysicsSystem.h"
#include "game/system/movement/GroundingSystem.h"
#include "game/component/movement/TransformComponent.h"
#include "game/actor/Player.h"
#include "game/GameManager.h"
#include "game/PauseManager.h"
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
#include "game/component/combat/ColliderComponent.h"
#include "game/component/combat/AttackComponent.h"
#include "game/constant/ModelId.h"
#include "game/constant/AnimationId.h"
#include "game/constant/ProjectileId.h"
#include "game/constant/EnemyType.h"
#include "game/component/EnemyTypeComponent.h"
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
#include "game/system/camera/DebugCameraSystem.h" // DEBUG: フリーカメラ（リリース時に削除）
#include "game/component/camera/CameraComponent.h"
#include "game/system/combat/TargetingSystem.h"
#include "game/component/combat/AimComponent.h"
#include "game/system/combat/ProjectileSystem.h"
#include "game/system/combat/ProjectileReflectSystem.h"
#include "game/system/combat/PlayerRangedAttackSystem.h"
#include "game/system/combat/PlayerAttackComboSystem.h"
#include "game/system/visual/PlayerChargeVisualsSystem.h"
#include "game/system/visual/CriticalVisualsSystem.h"
#include "game/system/camera/ChargeZoomSystem.h"
#include "game/system/camera/DamageShakeSystem.h"
#include "game/system/visual/MacAwakenEffectSystem.h"
#include "game/system/visual/BackgroundParticleSystem.h"
#include "game/system/visual/HardAuraVisualsSystem.h"
#include "game/system/visual/BattleStartSystem.h"
#include "game/ui/debug/DebugGizmoView.h"            // DEBUG: リリース時に削除
#include "game/ui/debug/DebugHUDView.h"              // DEBUG: リリース時に削除
#include "game/ui/ingame/PlayerHUDView.h"
#include "game/ui/ingame/EquipmentSlotView.h"
#include "game/ui/ingame/ObjectiveView.h"
#include "game/ui/ingame/InGameStatusView.h"
#include "game/ui/ingame/LowHealthVignetteView.h"
#include "game/ui/ingame/BossHUDView.h"
#include "game/ui/ingame/EnemyHealthBarView.h"
#include "core/interface/IPerformanceDataProvider.h" // DEBUG: リリース時に削除
#include "game/event/InGameEvents.h"

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
			setup.m_visuals.push_back({ handle, resolveProjectileRadius(resourceManager, setup.m_meta, handle) });
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
		setup.m_radius = resolveProjectileRadius(resourceManager, setup.m_meta, setup.m_handle);
		// モデル原点が見た目の中心とズレていると回転で円軌道を描くため、中心を求めて逆補正する
		setup.m_center = resourceManager.computeBoundingCenter(setup.m_handle);
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
	    PauseManager& pauseManager)
	    : m_camera{ camera }
	    , m_renderer{ renderer }
	    , m_animator{ animator }
	    , m_resourceManager{ resourceManager }
	    , m_inputProvider{ inputProvider }
	    , m_gameManager{ gameManager }
	    , m_pauseManager{ pauseManager }
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
		    m_effectFactory }
	{
		loadResources();
		spawnEntities();
		m_audioEventListener = std::make_unique<game::event::AudioEventListener>(m_eventBus, m_playerId);
		setupSystems();
		setupEvents();

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
		constexpr float FOG_START{ 3000.0f }; // ここから徐々に闇へ
		constexpr float FOG_END{ 9000.0f };   // ここで完全に闇へ溶ける
		m_camera.setNearFar(NEAR_CLIP, FAR_CLIP);
		if (screen)
			screen->setFog(true, VOID_R, VOID_G, VOID_B, FOG_START, FOG_END);

		// ライティングを有効化して立体感を出す。環境光は「模様が潰れない下限」を確保しつつ
		// 低めにして虚無の暗さを残し、上からの平行光で面の向きを分からせる
		auto* lighting{ core::base::ServiceLocator::get<core::iface::ILighting>() };
		if (lighting)
		{
			constexpr int AMBIENT_R{ 150 };
			constexpr int AMBIENT_G{ 160 };
			constexpr int AMBIENT_B{ 180 };
			lighting->setEnabled(true);
			lighting->setAmbient(AMBIENT_R, AMBIENT_G, AMBIENT_B);
			lighting->setDirectionalLight(core::Vector3{ -0.3f, -1.0f, 0.4f }, 255, 255, 255);
		}

		// DEBUG: 何かと不便なためリリースするときにfalseに変更すること
		// 3人称マウス視点のためカーソルを非表示にする
		m_inputProvider.setMouseCursorVisible(false);

		// DEBUG: ワールド空間デバッグ可視化・常時デバッグHUD（リリース時にまとめて削除）
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

		m_playerHUDView = std::make_unique<ui::ingame::PlayerHUDView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_componentManager);
		m_view.setPlayerHUDView(m_playerHUDView.get());

		m_equipmentSlotView = std::make_unique<ui::ingame::EquipmentSlotView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_fileEquipmentData,
		    m_resourceManager);
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

		m_lowHealthVignetteView = std::make_unique<ui::ingame::LowHealthVignetteView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_componentManager,
		    m_resourceManager);
		m_view.setLowHealthVignetteView(m_lowHealthVignetteView.get());

		m_bossHUDView = std::make_unique<ui::ingame::BossHUDView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_componentManager);
		m_view.setBossHUDView(m_bossHUDView.get());

		m_enemyHealthBarView = std::make_unique<ui::ingame::EnemyHealthBarView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_componentManager,
		    m_renderer);
		m_view.setEnemyHealthBarView(m_enemyHealthBarView.get());
	}

	InGame::~InGame() = default;

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
		core::log::info("Player[{}] HP={} ATK={} DEF={} SPD={} 攻撃範囲={} クールダウン={} 会心率={} 会心倍率={}",
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

		initializer.initializePlayer(m_playerData);
		m_playerId = m_factoryManager.getPlayerFactory().getPlayer().getId();

		// プレイヤー専用コンポーネント（CameraComponent、AimComponent、PlayerChargeComponent）
		// は Player.cpp のコンストラクタで初期化済

		// stageData.jsonのplayerStartを初期位置・初期向き（モデル・カメラyaw）へ反映する。
		// rotationYは度数法なのでラジアンへ変換する
		const auto& playerStart{ m_resourceManager.getStageMetadata().m_playerStart };
		auto& playerTransform{ m_componentManager.get<component::movement::TransformComponent>(m_playerId) };
		playerTransform.m_position = playerStart.m_position;

		const float startYawRad{ playerStart.m_rotationY * core::utility::DEG_TO_RAD };
		playerTransform.m_rotation.y = startYawRad;
		if (m_componentManager.has<component::camera::CameraComponent>(m_playerId))
			m_componentManager.get<component::camera::CameraComponent>(m_playerId).m_yaw = startYawRad;

		// 地面は stageData.json の props[] が持つ（単一の水平地面は坂と競合するため生成しない）
		initializer.initializeProps();

		// ステージ定義の点光源（青い道中・白銀のアリーナなどの明暗演出）
		initializer.initializeLights();

		// 生成される敵の追跡対象をプレイヤーに設定してからスポーンする
		m_enemySpawner.setTargetEntity(core::ecs::Entity(m_playerId));
		m_enemySpawner.spawnStageEnemies();

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
		    m_playerId);
		m_view.setBattleStartSystem(m_battleStartSystem);

		m_systemManager.registerSystem<game::system::movement::InputSystem>(m_componentManager, m_playerId, m_inputProvider, m_gameManager);
		// カメラ演出（Zoom/Shake）はCameraSystemより前に走らせ、合成結果をCameraEffectComponentへ書いておく
		m_systemManager.registerSystem<game::system::camera::ChargeZoomSystem>(m_componentManager, m_playerId);
		m_systemManager.registerSystem<game::system::camera::DamageShakeSystem>(m_componentManager, m_eventBus, m_playerId);
		// ボス覚醒演出（ズーム・シェイク・赤ビネット）。CameraSystemより前に走らせて演出チャンネルを書く。
		// 描画（赤ビネット）はInGameViewの描画フェーズから呼ぶためポインタを渡す
		auto* macAwakenEffect{ m_systemManager.registerSystem<game::system::visual::MacAwakenEffectSystem>(
			m_componentManager, m_eventBus,
			*core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
			*core::base::ServiceLocator::get<core::iface::IScreen>(),
			m_playerId) };
		m_view.setMacAwakenEffectSystem(macAwakenEffect);
		// カメラはMoveSystemより前に更新し、最新のyawで移動方向を計算させる
		m_systemManager.registerSystem<game::system::camera::CameraSystem>(m_componentManager, m_playerId, m_inputProvider, m_camera, m_gameManager);
		// DEBUG: デバッグモード時のフリーカメラ。CameraSystem直後・MoveSystemより前に走らせる（リリース時に削除）
		// シーンビュー凍結中に単独更新するためポインタも保持する
		m_debugCameraSystem = m_systemManager.registerSystem<game::system::camera::DebugCameraSystem>(
		    m_componentManager, m_playerId, m_inputProvider, m_camera, m_gameManager, m_pauseManager);
		m_systemManager.registerSystem<game::system::movement::MoveSystem>(m_componentManager, m_playerId, m_playerData.getMoveSpeed(), m_playerData.getDashMultiplier());
		// 照準の敵捕捉判定（カメラ更新後・描画前に走らせる）
		m_systemManager.registerSystem<game::system::combat::TargetingSystem>(m_componentManager);
		// 発射入力→弾生成（生成はPhysicsSystemより前でよい）。弾定義はjsonから取得する。
		// Window弾の見た目はビルボード（板に貼ったWindow画像）で描くので、その画像を先に読む
		auto projectileMeta{ m_resourceManager.getProjectileMetadata(constant::projectile_id::PLAYER_WINDOW) };
		const int windowBillboard{ projectileMeta.m_imageId.empty() ? -1 : m_resourceManager.loadImageById(projectileMeta.m_imageId) };

		// 装備ファイルのボーナスを弾定義へ反映する。
		// 飛距離は「弾速×寿命」で決まるため、弾速だけを上げると距離まで一緒に伸びてしまう。
		// 元の飛距離にボーナスを足したうえで、新しい弾速から寿命を逆算し、
		// 「速さ」と「距離」を別々のボーナスとして独立に効かせる
		const float baseProjectileRange{ projectileMeta.m_speed * projectileMeta.m_lifetime };
		projectileMeta.m_speed += m_playerData.getProjectileSpeedBonus();
		if (projectileMeta.m_speed > 0.0f)
			projectileMeta.m_lifetime = (baseProjectileRange + m_playerData.getProjectileRangeBonus()) / projectileMeta.m_speed;

		auto* rangedAttack{ m_systemManager.registerSystem<game::system::combat::PlayerRangedAttackSystem>(
			m_componentManager, m_playerId, m_projectileFactory, projectileMeta, windowBillboard) };
		// レティクルがクールダウンの残量を読むため、Viewへ参照を渡す
		m_view.setPlayerRangedAttackSystem(rangedAttack);
		m_systemManager.registerSystem<game::system::movement::PhysicsSystem>(m_componentManager, m_gameManager, m_playerData.getJumpForce(), m_playerData.getGravity(), m_playerData.getMaxFallSpeed());
		// 弾の寿命・再アーム・破棄（当たり判定するAttackSystemより前で再アームする）
		m_systemManager.registerSystem<game::system::combat::ProjectileSystem>(m_componentManager, m_entityManager, m_eventBus);
		// 敵弾をプレイヤーのWindow弾で跳ね返す（移動後・ダメージ判定AttackSystemより前に判定する）
		m_systemManager.registerSystem<game::system::combat::ProjectileReflectSystem>(m_componentManager);

		m_systemManager.registerSystem<game::system::visual::AnimationSystem>(m_componentManager, m_animator, m_eventBus);

		m_systemManager.registerSystem<game::system::combat::CollisionSystem>(m_componentManager);
		// 障害物の押し返し後に、床・坂の傾いた面へ足を乗せる（坂はAABBで表せないため専用処理）
		m_systemManager.registerSystem<game::system::movement::GroundingSystem>(m_componentManager);
		// AI行動分割：近接追跡型敵を駆動
		m_systemManager.registerSystem<game::system::ai::MeleeChaseAISystem>(m_componentManager);
		// AI行動分割：遠距離維持型敵を駆動
		m_systemManager.registerSystem<game::system::ai::RangeKeepAISystem>(m_componentManager);
		// 遠距離維持型敵の弾発射（Safariのタブ投擲）。見た目は3種のタブモデルからランダムに選ぶ
		auto tabSetup{ buildTabProjectileSetup(m_resourceManager) };
		m_systemManager.registerSystem<game::system::ai::EnemyRangedAttackSystem>(
		    m_componentManager, m_projectileFactory, tabSetup.m_meta, std::move(tabSetup.m_visuals));

		// ボス（Mac）のFSM駆動。遠距離はレインボー弾を扇状に、召喚はEnemySpawner経由で行う
		const auto rainbow{ buildRainbowSetup(m_resourceManager) };
		m_systemManager.registerSystem<game::system::ai::MacAISystem>(
		    m_componentManager, m_eventBus, m_projectileFactory, m_enemySpawner,
		    rainbow.m_meta, rainbow.m_handle, rainbow.m_radius, rainbow.m_center);

		// 敵がプレイヤーを発見した瞬間を検知（全敵共通）。発見演出のトリガーになる
		m_systemManager.registerSystem<game::system::ai::DetectionSystem>(m_componentManager, m_eventBus);

		// プレイヤーの近接攻撃入力をコンボの段数へ振り分ける（攻撃の成立はAttackSystem）
		m_systemManager.registerSystem<game::system::combat::PlayerAttackComboSystem>(
		    m_componentManager, m_playerId);

		m_systemManager.registerSystem<game::system::combat::AttackSystem>(
		    m_componentManager, m_eventBus, core::constant::SeType::AttackPlayer);
		m_systemManager.registerSystem<game::system::visual::HitEffectSystem>(m_componentManager, m_eventBus);
		// 死亡した敵の後始末（赤化＋ディゾルブ演出→Entity破棄＋モデルハンドルのプール返却）
		m_systemManager.registerSystem<game::system::combat::EnemyDeathSystem>(m_componentManager, m_entityManager, m_eventBus, m_enemySpawner, m_renderer);

		// プレイヤーの死亡演出（死亡アニメ→暗転）。完了時にシーン遷移用のイベントを発行する。
		// 描画（暗転）はInGameViewの描画フェーズから呼ぶためポインタを渡す
		auto* playerDeath{ m_systemManager.registerSystem<game::system::combat::PlayerDeathSystem>(
			m_componentManager, m_eventBus,
			*core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
			*core::base::ServiceLocator::get<core::iface::IScreen>(),
			m_playerId) };
		m_view.setPlayerDeathSystem(playerDeath);

		m_systemManager.registerSystem<game::system::visual::EffectSystem>(m_componentManager, m_eventBus, m_effectFactory);

		// 壁などの模様を流す（貼り方をずらすだけなので描画状態に影響しない）
		m_systemManager.registerSystem<game::system::visual::TextureScrollSystem>(m_componentManager);

		// 装着武器の装着先ボーンを解決する（解決はEntityごとに一度きり。描画はInGameView）
		m_systemManager.registerSystem<game::system::visual::WeaponAttachSystem>(m_componentManager, m_renderer);

		// LightComponentを持つエンティティの点光源を生成・追従させる（プレイヤーの携行灯など）
		if (auto* lighting{ core::base::ServiceLocator::get<core::iface::ILighting>() })
			m_systemManager.registerSystem<game::system::visual::LightSystem>(m_componentManager, *lighting);


		// プレイヤーの溜め攻撃の画面演出（集中線）。描画内容はSystemが持ち、
		// InGameViewには描画フェーズで呼び出させるためにポインタを渡す
		auto* chargeVisuals{ m_systemManager.registerSystem<game::system::visual::PlayerChargeVisualsSystem>(
			m_componentManager,
			*core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
			*core::base::ServiceLocator::get<core::iface::IScreen>(),
			m_playerId) };
		m_view.setPlayerChargeVisualsSystem(chargeVisuals);

		// クリティカルの瞬間に弾ける集中線
		auto* criticalVisuals{ m_systemManager.registerSystem<game::system::visual::CriticalVisualsSystem>(
			m_componentManager,
			m_eventBus,
			*core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
			*core::base::ServiceLocator::get<core::iface::IScreen>(),
			m_playerId) };
		m_view.setCriticalVisualsSystem(criticalVisuals);

		// 虚空を流れるデータの光跡（背景の奥行きと動きを作る）
		auto* backgroundParticles{ m_systemManager.registerSystem<game::system::visual::BackgroundParticleSystem>(
			m_componentManager, m_playerId, m_renderer, m_resourceManager) };
		m_view.setBackgroundParticleSystem(backgroundParticles);

		// Hardの敵を包む赤いオーラ（強化されていることを戦闘中に伝える）
		auto* hardAura{ m_systemManager.registerSystem<game::system::visual::HardAuraVisualsSystem>(
			m_componentManager, m_renderer, m_resourceManager,
			m_gameManager.getDifficulty() == core::data::Difficulty::Hard) };
		m_view.setHardAuraVisualsSystem(hardAura);

		// 敵の発見演出（頭上の通知バッジ）。描画内容はSystemが持ち、Viewが描画フェーズで呼ぶ
		auto* detectionAlert{ m_systemManager.registerSystem<game::system::visual::DetectionAlertVisualsSystem>(
			m_componentManager,
			m_eventBus,
			m_renderer,
			*core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
			*core::base::ServiceLocator::get<core::iface::IScreen>(),
			m_resourceManager) };
		m_view.setDetectionAlertVisualsSystem(detectionAlert);

		// 敵に与えたダメージ量の表示。描画内容はSystemが持ち、Viewが描画フェーズで呼ぶ
		auto* damagePopup{ m_systemManager.registerSystem<game::system::visual::DamagePopupSystem>(
			m_componentManager,
			m_eventBus,
			m_renderer,
			*core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
			*core::base::ServiceLocator::get<core::iface::IScreen>()) };
		m_view.setDamagePopupSystem(damagePopup);

		// 攻撃予兆（地面の攻撃範囲サークル）。描画内容はSystemが持ち、Viewが3D描画フェーズで呼ぶ
		auto* attackTelegraph{ m_systemManager.registerSystem<game::system::visual::AttackTelegraphVisualsSystem>(
			m_componentManager, m_renderer) };
		m_view.setAttackTelegraphVisualsSystem(attackTelegraph);

		// 汎用の攻撃予兆（TelegraphComponent駆動：円・扇）。ボスの溜め攻撃などが使う
		auto* telegraph{ m_systemManager.registerSystem<game::system::visual::TelegraphVisualsSystem>(
			m_componentManager, m_renderer) };
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

				// クリティカルの瞬間に一拍止めて会心の手応えを作る。
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
			    // メニュー操作用にカーソルを戻してからシーンを切り替える
			    m_inputProvider.setMouseCursorVisible(true);
			    auto* sceneManager{ core::base::ServiceLocator::get<game::scene::SceneManager>() };
			    sceneManager->changeScene(game::scene::SceneType::Result);
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
			    // メニュー操作用にカーソルを戻してからシーンを切り替える
			    m_inputProvider.setMouseCursorVisible(true);
			    auto* sceneManager{ core::base::ServiceLocator::get<game::scene::SceneManager>() };
			    sceneManager->changeScene(game::scene::SceneType::Result);
		    }));
	}

	void InGame::spawnBoss()
	{
		const auto& macSpawn{ m_resourceManager.getStageMetadata().m_mac };
		if (macSpawn.m_type.empty())
			return; // ボス未定義のステージなら何もしない（勝利条件が成立しなくなる点は許容）

		m_macId = m_enemySpawner.spawn(constant::toEnemyType(macSpawn.m_type), macSpawn.m_position,
		    macSpawn.m_rotationY);
		core::log::info("雑魚を全滅：ボスが出現しました (EntityId={})", m_macId);

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

		// DEBUG: F1キーでデバッグモード（フリーカメラ）のON/OFFを切り替える（リリース時に削除）
		if (m_inputProvider.isKeyPressed(core::input::KeyCode::F1))
		{
			m_gameManager.toggleDebugMode();
			if (m_gameManager.isDebugMode())
				core::log::info("DEBUG: デバッグモードON");
			else
				core::log::info("DEBUG: デバッグモードOFF");
		}

		// DEBUG: F2キーでシーンビュー（時間停止＋フリーカメラ）のON/OFFを切り替える（リリース時に削除）
		if (m_inputProvider.isKeyPressed(core::input::KeyCode::F2))
		{
			m_pauseManager.toggle(PauseReason::DebugSceneView);
			if (m_pauseManager.isPausedBy(PauseReason::DebugSceneView))
				core::log::info("DEBUG: シーンビューON（時間停止）");
			else
				core::log::info("DEBUG: シーンビューOFF");
		}

		// DEBUG: F3キーで連続ジャンプ（空中浮上）のON/OFFを切り替える（リリース時に削除）
		if (m_inputProvider.isKeyPressed(core::input::KeyCode::F3))
		{
			m_gameManager.toggleContinuousJump();
			if (m_gameManager.isContinuousJumpEnabled())
				core::log::info("DEBUG: 連続ジャンプON（空中浮上可）");
			else
				core::log::info("DEBUG: 連続ジャンプOFF（接地単発）");
		}

		// DEBUG: シーンビュー凍結中はゲームロジックを止め、フリーカメラだけを更新する（リリース時に削除）
		if (m_pauseManager.isPausedBy(PauseReason::DebugSceneView))
		{
			if (m_debugCameraSystem)
				m_debugCameraSystem->update(deltaTime);
			return;
		}

		// ヒットストップ中はSystemへ渡す時間に倍率を掛ける（0なら何も進まない）。
		// 経過時間の計測もここへ揃える。止まっている間もタイマーだけ進むと、
		// 画面が止まっているのに右上の秒数だけ動いて不自然になる
		const float scaledDeltaTime{ m_hitStop.apply(deltaTime) };

		// 開始演出（READY）の間はまだ動けないので、クリアタイムの計測も始めない
		if (m_battleStartSystem == nullptr || !m_battleStartSystem->isPreparing())
			m_elapsedTime += scaledDeltaTime;
		m_systemManager.update(scaledDeltaTime);

		// DEBUG: Tキーでプレイヤー位置にテストエフェクト（Enemy_Spawn）を再生する（テスト後に削除）
		if (m_inputProvider.isKeyPressed(core::input::KeyCode::T))
		{
			const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(m_playerId) };
			m_effectFactory.play(core::constant::EffectType::Enemy_HitWindow, transform.m_position, {});
			core::log::info("エフェクトが再生");
		}
	}

	void InGame::draw()
	{
		// DEBUG: FPS計測とCPU/メモリ取得。シーンビュー凍結中もdrawは呼ばれるため計測は続く
		// （リリース時に削除）
		if (m_debugHUDView)
			m_debugHUDView->updateOnRenderFrame();

		// 描画は InGameView へ委譲する。ボスが召喚する雑魚も実行時に増えるため、
		// スポーン時のスナップショットではなく EnemyFactory が持つ最新の敵一覧を渡す
		// 残り雑魚はボス出現条件そのものなので、開始時スナップショットの生き残り数を渡す
		// （ボスが召喚する雑魚は条件に含めない）
		m_view.draw(m_playerId, static_cast<int>(m_stageEnemyIds.size()), m_macId, m_elapsedTime);
	}

	void InGame::saveResultData	(bool isVictory) noexcept
	{
		core::data::ResultData result{};
		result.m_isVictory        = isVictory;
		result.m_elapsedTime      = m_elapsedTime;
		result.m_killCount        = m_killCount;
		result.m_totalDamageTaken = m_totalDamageTaken;

		for (int i{0}; i < data::FileEquipmentData::MAX_SLOTS; ++i)
		{
			if (m_fileEquipmentData.hasSelection(i))
				result.m_usedFiles.push_back(m_fileEquipmentData.getFilePath(i));
		}

		m_gameManager.setResultData(result);
	}
} // namespace game::scene