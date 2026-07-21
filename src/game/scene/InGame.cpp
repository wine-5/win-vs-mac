#include "InGame.h"

/* core層 */
#include "core/interface/ILogger.h"
#include "core/interface/IEffectFactory.h"
#include "core/interface/IAudioManager.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/utility/Color.h"
#include "core/base/ServiceLocator.h"
#include "core/constant/JobType.h"
#include "core/constant/SeType.h"
#include "core/data/ResultData.h"
/* game層 */
#include "game/factory/FactoryInitializer.h"
#include "game/system/InputSystem.h"
#include "game/system/MoveSystem.h"
#include "game/system/PhysicsSystem.h"
#include "game/component/TransformComponent.h"
#include "game/actor/Player.h"
#include "game/GameManager.h"
#include "game/PauseManager.h"
#include "game/component/RenderComponent.h"
#include "game/component/HealthComponent.h"
#include "game/component/HitEffectComponent.h"
#include "game/system/AnimationSystem.h"
#include "game/system/CollisionSystem.h"
#include "game/system/HitEffectSystem.h"
#include "game/system/EffectSystem.h"
#include "core/interface/IEffectFactory.h"
#include "game/system/AttackSystem.h"
#include "game/component/ColliderComponent.h"
#include "game/component/AttackComponent.h"
#include "game/constant/ModelId.h"
#include "game/constant/AnimationId.h"
#include "game/constant/ProjectileId.h"
#include "game/constant/EnemyType.h"
#include "game/scene/SceneManager.h"
#include "game/scene/SceneType.h"
#include "game/system/ai/MeleeChaseAISystem.h"
#include "game/system/ai/RangeKeepAISystem.h"
#include "game/system/ai/EnemyRangedAttackSystem.h"
#include "game/system/ai/BossAISystem.h"
#include "game/component/AIComponent.h"
#include "game/system/CameraSystem.h"
#include "game/system/DebugCameraSystem.h" // DEBUG: フリーカメラ（リリース時に削除）
#include "game/component/CameraComponent.h"
#include "game/system/TargetingSystem.h"
#include "game/component/AimComponent.h"
#include "game/system/ProjectileSystem.h"
#include "game/system/ProjectileReflectSystem.h"
#include "game/system/PlayerRangedAttackSystem.h"
#include "game/system/ProjectileWindowSystem.h"
#include "game/system/PlayerChargeVisualsSystem.h"
#include "game/system/ChargeZoomSystem.h"
#include "game/system/DamageShakeSystem.h"
#include "game/system/BossAwakenEffectSystem.h"
#include "game/ui/debug/DebugGizmoView.h"            // DEBUG: リリース時に削除
#include "game/ui/debug/DebugHUDView.h"              // DEBUG: リリース時に削除
#include "core/interface/IPerformanceDataProvider.h" // DEBUG: リリース時に削除
#include "core/interface/IWindowFactory.h"
#include "game/event/InGameEvents.h"
#include "game/utility/ExtensionBonusCalculator.h"

/* 標準のインクルード */
#include <cassert>
#include <stdexcept>
#include <cmath>
#include <array>
#include <vector>
#include <string_view>
#include <utility>

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
	    , m_factoryManager{ m_entityManager, m_componentManager, m_resourceManager }
	    , m_enemySpawner{ m_factoryManager, m_componentManager, m_resourceManager }
	    , m_projectileFactory{ m_entityManager, m_componentManager }
	    , m_playerData{ game::data::PlayerData::fromMetadata(m_resourceManager.getMetadata(constant::model_id::PLAYER).value()) }
	    , m_effectFactory{ *core::base::ServiceLocator::get<core::iface::IEffectFactory>() }
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

		// 背景（空）をUnityのデフォルトスカイブルー風にする
		constexpr int SKY_BLUE_R{ 135 };
		constexpr int SKY_BLUE_G{ 206 };
		constexpr int SKY_BLUE_B{ 235 };
		auto* screen{ core::base::ServiceLocator::get<core::iface::IScreen>() };
		if (screen)
			screen->setBackgroundColor(SKY_BLUE_R, SKY_BLUE_G, SKY_BLUE_B);

		// DEBUG: 何かと不便なためリリースするときにfalseに変更すること
		// 3人称マウス視点のためカーソルを非表示にする
		m_inputProvider.setMouseCursorVisible(true);

		// DEBUG: ワールド空間デバッグ可視化・常時デバッグHUD（リリース時にまとめて削除）
		m_debugGizmoView = std::make_unique<ui::debug::DebugGizmoView>(m_componentManager, m_renderer);
		m_debugHUDView = std::make_unique<ui::debug::DebugHUDView>(
		    *core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
		    *core::base::ServiceLocator::get<core::iface::IScreen>(),
		    m_componentManager,
		    m_gameManager,
		    m_pauseManager,
		    *core::base::ServiceLocator::get<core::iface::IPerformanceDataProvider>(),
		    m_effectFactory);
		m_view.setDebugGizmoView(m_debugGizmoView.get());
		m_view.setDebugHUDView(m_debugHUDView.get());
	}

	void InGame::loadResources()
	{
		// 先にモデルをロードして自動計算を実行
		m_resourceManager.loadModelById(constant::model_id::PLAYER);

		// モデルロード後に再度メタデータを取得してPlayerDataを更新
		auto playerMeta{ m_resourceManager.getMetadata(constant::model_id::PLAYER) };
		if (!playerMeta.has_value())
		{
			LOG("ERROR: Playerのメタデータが見つかりません");
			throw std::runtime_error("Playerのメタデータの読み込みに失敗しました");
		}
		assert(playerMeta.has_value() && "Playerのメタデータが見つかりません");
		m_playerData = game::data::PlayerData::fromMetadata(playerMeta.value());
	}

	void InGame::spawnEntities()
	{
		game::factory::FactoryInitializer initializer(m_factoryManager, m_resourceManager);

		// 職業パラメータをPlayerDataに反映
		const auto& jobSelectionData{ m_gameManager.getJobSelectionData() };
		if (jobSelectionData.hasJobSelected())
		{
			const auto jobType{ jobSelectionData.getSelectedJobType() };
			const auto jobInfo{ m_resourceManager.getJobInfo(jobType) };
			m_playerData.applyJobParameters(jobInfo.m_hp, jobInfo.m_atk, jobInfo.m_def, jobInfo.m_spd);
		}

		// 拡張子ボーナスをPlayerDataに反映
		for (int i{ 0 }; i < data::FileEquipmentData::MAX_SLOTS; ++i)
		{
			if (m_fileEquipmentData.hasSelection(i))
			{
				const auto bonus{ utility::ExtensionBonusCalculator::calculate(
					m_fileEquipmentData.getExtensionType(i)) };
				m_playerData.applyExtensionBonus(bonus);
			}
		}

		initializer.initializePlayer(m_playerData);
		m_playerId = m_factoryManager.getPlayerFactory().getPlayer().getId();

		// プレイヤー専用コンポーネント（CameraComponent、AimComponent、PlayerChargeComponent）
		// は Player.cpp のコンストラクタで初期化済

		m_groundId = initializer.initializeGround();

		// 生成される敵の追跡対象をプレイヤーに設定してからスポーンする
		m_enemySpawner.setTargetEntity(core::ecs::Entity(m_playerId));
		m_enemySpawner.spawnStageEnemies();

		// ボス（Mac）をステージ定義の boss 位置に生成する。撃破判定用にIDを保持する。
		// テスト時は stageData.json の boss.position を近くにすればすぐ戦える
		const auto& bossSpawn{ m_resourceManager.getStageMetadata().m_boss };
		if (!bossSpawn.m_type.empty())
			m_bossId = m_enemySpawner.spawn(constant::toEnemyType(bossSpawn.m_type), bossSpawn.m_position);
	}

	void InGame::setupSystems()
	{
		// システム登録
		m_systemManager.registerSystem<game::system::InputSystem>(m_componentManager, m_playerId, m_inputProvider, m_gameManager);
		// カメラ演出（Zoom/Shake）はCameraSystemより前に走らせ、合成結果をCameraEffectComponentへ書いておく
		m_systemManager.registerSystem<game::system::ChargeZoomSystem>(m_componentManager, m_playerId);
		m_systemManager.registerSystem<game::system::DamageShakeSystem>(m_componentManager, m_eventBus, m_playerId);
		// ボス覚醒演出（ズーム・シェイク・赤ビネット）。CameraSystemより前に走らせて演出チャンネルを書く。
		// 描画（赤ビネット）はInGameViewの描画フェーズから呼ぶためポインタを渡す
		auto* bossAwakenEffect{ m_systemManager.registerSystem<game::system::BossAwakenEffectSystem>(
			m_componentManager, m_eventBus,
			*core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
			*core::base::ServiceLocator::get<core::iface::IScreen>(),
			m_playerId) };
		m_view.setBossAwakenEffectSystem(bossAwakenEffect);
		// カメラはMoveSystemより前に更新し、最新のyawで移動方向を計算させる
		m_systemManager.registerSystem<game::system::CameraSystem>(m_componentManager, m_playerId, m_inputProvider, m_camera, m_gameManager);
		// DEBUG: デバッグモード時のフリーカメラ。CameraSystem直後・MoveSystemより前に走らせる（リリース時に削除）
		// シーンビュー凍結中に単独更新するためポインタも保持する
		m_debugCameraSystem = m_systemManager.registerSystem<game::system::DebugCameraSystem>(
		    m_componentManager, m_playerId, m_inputProvider, m_camera, m_gameManager, m_pauseManager);
		m_systemManager.registerSystem<game::system::MoveSystem>(m_componentManager, m_playerId, m_playerData.getMoveSpeed(), m_playerData.getDashMultiplier());
		// 照準の敵捕捉判定（カメラ更新後・描画前に走らせる）
		m_systemManager.registerSystem<game::system::TargetingSystem>(m_componentManager);
		// 発射入力→弾生成（生成はPhysicsSystemより前でよい）。弾定義はjsonから取得する
		const auto& projectileMeta{ m_resourceManager.getProjectileMetadata(constant::projectile_id::PLAYER_WINDOW) };
		m_systemManager.registerSystem<game::system::PlayerRangedAttackSystem>(m_componentManager, m_playerId, m_projectileFactory,
		    projectileMeta);
		m_systemManager.registerSystem<game::system::PhysicsSystem>(m_componentManager);
		// 弾の寿命・再アーム・破棄（当たり判定するAttackSystemより前で再アームする）
		m_systemManager.registerSystem<game::system::ProjectileSystem>(m_componentManager, m_entityManager, m_eventBus);
		// 敵弾をプレイヤーのWindow弾で跳ね返す（移動後・ダメージ判定AttackSystemより前に判定する）
		m_systemManager.registerSystem<game::system::ProjectileReflectSystem>(m_componentManager);

		// 弾の見た目として実OSウィンドウを追従させる（移動後の位置を射影するためPhysicsSystemより後）
		auto* windowFactory{ core::base::ServiceLocator::get<core::iface::IWindowFactory>() };
		if (windowFactory)
			m_projectileWindowManager = windowFactory->createProjectileWindowManager();
		if (m_projectileWindowManager)
			m_systemManager.registerSystem<game::system::ProjectileWindowSystem>(m_componentManager, m_renderer, *m_projectileWindowManager);
		m_systemManager.registerSystem<game::system::AnimationSystem>(m_componentManager, m_animator, m_eventBus);

		m_systemManager.registerSystem<game::system::CollisionSystem>(m_componentManager);
		// AI行動分割：近接追跡型敵を駆動
		m_systemManager.registerSystem<game::system::ai::MeleeChaseAISystem>(m_componentManager);
		// AI行動分割：遠距離維持型敵を駆動
		m_systemManager.registerSystem<game::system::ai::RangeKeepAISystem>(m_componentManager);
		// 遠距離維持型敵の弾発射（Safariのタブ投擲）。見た目は3種のタブモデルからランダムに選ぶ。
		// 当たり判定半径は projectileData.json の radius が 0 ならモデル実寸から自動計算、0以外なら手動指定
		const auto& tabProjectileMeta{ m_resourceManager.getProjectileMetadata(constant::projectile_id::ENEMY_SAFARI_TAB) };
		const std::array<std::string_view, 3> tabModelIds{
			constant::model_id::TAB_STORAGE_FULL,
			constant::model_id::TAB_SAFARI_ERROR,
			constant::model_id::TAB_XCODE_BUILDING
		};
		std::vector<game::system::ai::RangedProjectileVisual> tabVisuals{};
		for (const auto tabModelId : tabModelIds)
		{
			const int handle{ m_resourceManager.loadModelById(tabModelId) };
			const float radius{ tabProjectileMeta.m_radius > 0.0f
				                    ? tabProjectileMeta.m_radius
				                    : m_resourceManager.computeBoundingRadius(handle, tabProjectileMeta.m_scale) };
			tabVisuals.push_back({ handle, radius });
		}
		m_systemManager.registerSystem<game::system::ai::EnemyRangedAttackSystem>(
		    m_componentManager, m_projectileFactory, tabProjectileMeta, std::move(tabVisuals));

		// ボス（Mac）のFSM駆動。遠距離はレインボー弾を扇状に、召喚はEnemySpawner経由で行う。
		// レインボーの当たり判定半径は projectileData.json の radius が 0 ならモデル実寸から自動計算
		const auto& rainbowMeta{ m_resourceManager.getProjectileMetadata(constant::projectile_id::BOSS_RAINBOW) };
		const int rainbowHandle{ m_resourceManager.loadModelById(constant::model_id::BOSS_RAINBOW_WHEEL) };
		const float rainbowRadius{ rainbowMeta.m_radius > 0.0f
			                           ? rainbowMeta.m_radius
			                           : m_resourceManager.computeBoundingRadius(rainbowHandle, rainbowMeta.m_scale) };
		// モデル原点が見た目の中心とズレていると回転で円軌道を描くため、中心を求めて逆補正する
		const core::Vector3 rainbowCenter{ m_resourceManager.computeBoundingCenter(rainbowHandle) };

		m_systemManager.registerSystem<game::system::ai::BossAISystem>(
		    m_componentManager, m_eventBus, m_projectileFactory, m_enemySpawner,
		    rainbowMeta, rainbowHandle, rainbowRadius, rainbowCenter);

		// ジョブに応じた攻撃SEタイプを決定してAttackSystemに渡す
		core::constant::SeType playerAttackSeType{ core::constant::SeType::None };
		const auto& jobData{ m_gameManager.getJobSelectionData() };
		if (jobData.hasJobSelected())
		{
			switch (jobData.getSelectedJobType())
			{
			case core::constant::JobType::Warrior: playerAttackSeType = core::constant::SeType::AttackWarrior; break;
			case core::constant::JobType::Mage:    playerAttackSeType = core::constant::SeType::AttackFire;    break;
			case core::constant::JobType::Ninja:   playerAttackSeType = core::constant::SeType::AttackNinja;   break;
			}
		}
		m_systemManager.registerSystem<game::system::AttackSystem>(m_componentManager, m_eventBus, playerAttackSeType);
		m_systemManager.registerSystem<game::system::HitEffectSystem>(m_componentManager, m_eventBus);

		m_systemManager.registerSystem<game::system::EffectSystem>(m_componentManager, m_eventBus, m_effectFactory);

		// プレイヤーの溜め攻撃の画面演出（集中線）。描画内容はSystemが持ち、
		// InGameViewには描画フェーズで呼び出させるためにポインタを渡す
		auto* chargeVisuals{ m_systemManager.registerSystem<game::system::PlayerChargeVisualsSystem>(
			m_componentManager,
			*core::base::ServiceLocator::get<core::iface::IUIRenderer>(),
			*core::base::ServiceLocator::get<core::iface::IScreen>(),
			m_playerId) };
		m_view.setPlayerChargeVisualsSystem(chargeVisuals);
	}

	void InGame::setupEvents()
	{
		// Hitイベントの購読
		m_eventBus.subscribe<event::AttackHitEvent>([this](const event::AttackHitEvent& e)
			{
				// 被ダメージ追跡（プレイヤーが攻撃を受けた場合）
				if (e.m_targetId == m_playerId)
					m_totalDamageTaken += e.m_damage;
			});
			// プレイヤー死亡イベントの購読
				m_eventBus.subscribe<event::PlayerDeadEvent>([this](const event::PlayerDeadEvent&)
					{
						if (m_componentManager.has<component::RenderComponent>(m_playerId))
						{
							auto& render{ m_componentManager.get<component::RenderComponent>(m_playerId) };
							render.m_isVisible = false;
						}

						saveResultData(false);
			            // メニュー操作用にカーソルを戻してからシーンを切り替える
			            m_inputProvider.setMouseCursorVisible(true);
			            auto* sceneManager{ core::base::ServiceLocator::get<game::scene::SceneManager>() };
						sceneManager->changeScene(game::scene::SceneType::Result); });

				// 敵の死亡イベントの購読
				m_eventBus.subscribe<event::EnemyDeadEvent>([this](const event::EnemyDeadEvent& e)
					{
						if (m_componentManager.has<component::RenderComponent>(e.m_entityId))
						{
							auto& render{ m_componentManager.get<component::RenderComponent>(e.m_entityId) };
							render.m_isVisible = false;
						}

			            // AIを停止する。これをしないと死亡（非表示）後も移動や弾発射が続き、
			            // 「見えないのにSafariがタブをまだPlayerに投げてくる」状態になる
			            if (m_componentManager.has<component::AIComponent>(e.m_entityId))
				            m_componentManager.get<component::AIComponent>(e.m_entityId).m_isActive = false;

			            m_killCount++;

			            // 勝利条件はボス撃破のみ（ボスは雑魚を絶えず生成するため全滅判定にしない）
			            if (e.m_entityId == m_bossId)
			            {
							saveResultData(true);
				            // メニュー操作用にカーソルを戻してからシーンを切り替える
				            m_inputProvider.setMouseCursorVisible(true);
				            auto* sceneManager{ core::base::ServiceLocator::get<game::scene::SceneManager>() };
							sceneManager->changeScene(game::scene::SceneType::Result);
						} });
	}

	void InGame::update(float deltaTime)
	{
		// DEBUG: シーンビュー凍結中もFPS計測やCPU/メモリ取得は続ける（リリース時に削除）
		if (m_debugHUDView)
			m_debugHUDView->update();

		// DEBUG: F1キーでデバッグモード（フリーカメラ）のON/OFFを切り替える（リリース時に削除）
		if (m_inputProvider.isKeyPressed(core::input::KeyCode::F1))
		{
			m_gameManager.toggleDebugMode();
			if (m_gameManager.isDebugMode())
				LOG("DEBUG: デバッグモードON");
			else
				LOG("DEBUG: デバッグモードOFF");
		}

		// DEBUG: F2キーでシーンビュー（時間停止＋フリーカメラ）のON/OFFを切り替える（リリース時に削除）
		if (m_inputProvider.isKeyPressed(core::input::KeyCode::F2))
		{
			m_pauseManager.toggle(PauseReason::DebugSceneView);
			if (m_pauseManager.isPausedBy(PauseReason::DebugSceneView))
				LOG("DEBUG: シーンビューON（時間停止）");
			else
				LOG("DEBUG: シーンビューOFF");
		}

		// DEBUG: シーンビュー凍結中はゲームロジックを止め、フリーカメラだけを更新する（リリース時に削除）
		if (m_pauseManager.isPausedBy(PauseReason::DebugSceneView))
		{
			if (m_debugCameraSystem)
				m_debugCameraSystem->update(deltaTime);
			m_inputProvider.updatePreviousState();
			return;
		}

		m_elapsedTime += deltaTime;
		m_systemManager.update(deltaTime);

		// DEBUG: Tキーでプレイヤー位置にテストエフェクト（Enemy_Spawn）を再生する（テスト後に削除）
		if (m_inputProvider.isKeyPressed(core::input::KeyCode::T))
		{
			const auto& transform{ m_componentManager.get<component::TransformComponent>(m_playerId) };
			m_effectFactory.play(core::constant::EffectType::Enemy_HitWindow, transform.m_position);
			LOG("エフェクトが再生");
		}

		// フレーム最後に入力状態を更新
		m_inputProvider.updatePreviousState();
	}

	void InGame::draw()
	{
		// 描画は InGameView へ委譲する。ボスが召喚する雑魚も実行時に増えるため、
		// スポーン時のスナップショットではなく EnemyFactory が持つ最新の敵一覧を渡す
		m_view.draw(m_playerId, m_groundId, m_factoryManager.getEnemyFactory().getEnemyIds());
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