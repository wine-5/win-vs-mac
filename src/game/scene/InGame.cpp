#include "InGame.h"

/* core層 */
#include "core/interface/ILogger.h"
#include "core/utility/Color.h"
#include "core/base/ServiceLocator.h"

/* game層 */
#include "game/factory/FactoryInitializer.h"
#include "game/system/InputSystem.h"
#include "game/system/MoveSystem.h"
#include "game/system/PhysicsSystem.h"
#include "game/component/TransformComponent.h"
#include "game/actor/Player.h"
#include "game/component/RenderComponent.h"
#include "game/component/HealthComponent.h"
#include "game/component/HitEffectComponent.h"
#include "game/system/AnimationSystem.h"
#include "game/system/HitEffectSystem.h"
#include "game/system/CollisionSystem.h"
#include "game/system/AttackSystem.h"
#include "game/component/ColliderComponent.h"
#include "game/constant/ModelId.h"
#include "game/scene/SceneManager.h"
#include "game/scene/SceneType.h"
#include "game/system/AISystem.h"
#include "game/component/AIComponent.h"
#include "game/event/InGameEvents.h"
#include "game/utility/ExtensionBonusCalculator.h"

/* 標準のインクルード */
#include <cassert>
#include <stdexcept>

namespace game::scene
{
	InGame::InGame(core::iface::ICamera& camera,
		core::iface::IRenderer& renderer,
		core::iface::IAnimator& animator,
		core::iface::IResourceManager& resourceManager,
		core::iface::IInputProvider& inputProvider,
		data::FileEquipmentData& fileEquipmentData)
		: m_camera{ camera }, m_renderer{ renderer }, m_animator{ animator }, m_resourceManager{ resourceManager }, m_inputProvider{ inputProvider }, m_fileEquipmentData{ fileEquipmentData }, m_factoryManager{ m_entityManager, m_componentManager, m_resourceManager }, m_playerData{ game::data::PlayerData::fromMetadata(
																																																																  m_resourceManager.getMetadata(constant::model_id::PLAYER).value()) }
	{
		loadResources();
		spawnEntities();
		setupSystems();
		setupEvents();
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

		// テストとして表示するようにベースパラメータを保存
		const float baseHp{ m_playerData.getMaxHp() };
		const float baseAtk{ m_playerData.getAttackPower() };
		const float baseDef{ m_playerData.getDefence() };
		const float baseSpd{ m_playerData.getMoveSpeed() };
		const float baseRange{ m_playerData.getAttackRange() };

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
		// テスト用に表示
		LOG("=== Player パラメータ ===");
		LOG("  HP        : %.1f -> %.1f", baseHp, m_playerData.getMaxHp());
		LOG("  ATK       : %.1f -> %.1f", baseAtk, m_playerData.getAttackPower());
		LOG("  DEF       : %.1f -> %.1f", baseDef, m_playerData.getDefence());
		LOG("  SPD       : %.1f -> %.1f", baseSpd, m_playerData.getMoveSpeed());
		LOG("  ATK Range : %.1f -> %.1f", baseRange, m_playerData.getAttackRange());
		LOG("========================");

		initializer.initializePlayer(m_playerData);
		m_playerId = m_factoryManager.getPlayerFactory().getPlayer().getId();

		m_groundId = initializer.initializeGround();

		m_enemyId = initializer.initializeEnemy();
		// Enemyの初期位置をずらす
		auto& enemyTransform{ m_componentManager.get<component::TransformComponent>(m_enemyId) };
		enemyTransform.m_position.x = 100.0f;
		enemyTransform.m_position.z = 100.0f;

		auto& ai = m_componentManager.get<component::AIComponent>(m_enemyId);
		ai.m_targetEntity = core::ecs::Entity(m_playerId);
	}

	void InGame::setupSystems()
	{
		// アニメーションハンドルをロード
		// int idleAnimHandle = m_resourceManager.loadModelById(m_playerData.getIdleAnimPath());
		// int walkAnimHandle = m_resourceManager.loadModelById(m_playerData.getWalkAnimPath());

		// システム登録
		m_systemManager.registerSystem<game::system::InputSystem>(m_componentManager, m_playerId, m_inputProvider);
		m_systemManager.registerSystem<game::system::MoveSystem>(m_componentManager, m_playerId, m_playerData.getMoveSpeed());
		m_systemManager.registerSystem<game::system::PhysicsSystem>(m_componentManager);
		// m_systemManager.registerSystem<game::system::AnimationSystem>(
		//	m_componentManager,
		//	m_factoryManager.getPlayerFactory().getPlayer().getId(),
		//	m_animator,
		//	idleAnimHandle,
		//	walkAnimHandle);

		m_systemManager.registerSystem<game::system::CollisionSystem>(m_componentManager);
		m_systemManager.registerSystem<game::system::AISystem>(m_componentManager);
		m_systemManager.registerSystem<game::system::AttackSystem>(m_componentManager, m_eventBus);
		m_systemManager.registerSystem<game::system::HitEffectSystem>(m_componentManager);
	}

	void InGame::setupEvents()
	{
		// Hitイベントの購読
		m_eventBus.subscribe<event::AttackHitEvent>([this](const event::AttackHitEvent& e)
			{
				LOG("AttackHit: 攻撃者のId=%u 被攻撃者のId=%u ダメージ=%.1f",
					e.m_attackerId, e.m_targetId, e.m_damage);

				// ヒットエフェクト開始
				if (m_componentManager.has<component::HitEffectComponent>(e.m_targetId))
				{
					auto& effect{ m_componentManager.get<component::HitEffectComponent>(e.m_targetId) };
					effect.m_isActive = true;
					effect.m_durationTimer = effect.m_duration;
					effect.m_blinkTimer = effect.m_blinkInterval;
				} });

				// プレイヤー死亡イベントの購読
				m_eventBus.subscribe<event::PlayerDeadEvent>([this](const event::PlayerDeadEvent&)
					{
						LOG("PlayerDead: プレイヤーが死亡しました");
						if (m_componentManager.has<component::RenderComponent>(m_playerId))
						{
							auto& render{ m_componentManager.get<component::RenderComponent>(m_playerId) };
							render.m_isVisible = false;
						}

						auto* sceneManager{ core::base::ServiceLocator::get<game::scene::SceneManager>() };
						sceneManager->changeScene(game::scene::SceneType::Result); });

				// 敵の死亡イベントの購読
				m_eventBus.subscribe<event::EnemyDeadEvent>([this](const event::EnemyDeadEvent& e)
					{
						LOG("敵が死んだ：Id=%u", e.m_entityId);
						if (m_componentManager.has<component::RenderComponent>(e.m_entityId))
						{
							auto& render{ m_componentManager.get<component::RenderComponent>(e.m_entityId) };
							render.m_isVisible = false;
						}

						// 敵が全滅しているかチェック
						auto enemies{ m_componentManager.getAllEntities<component::AIComponent>() };
						bool allDead{ true };
						for (auto enemyId : enemies)
						{
							auto& health{ m_componentManager.get<component::HealthComponent>(enemyId) };
							if (!health.m_isDead)
							{
								allDead = false;
								break;
							}
						}
						if (allDead)
						{
							auto* sceneManager{ core::base::ServiceLocator::get<game::scene::SceneManager>() };
							sceneManager->changeScene(game::scene::SceneType::Result);
						} });
	}

	void InGame::update(float deltaTime)
	{
		// デバッグ用：Rキーでリザルト画面へ
		if (m_inputProvider.isKeyPressed(core::input::KeyCode::R))
		{
			LOG("INFO: Rキーでリザルト画面へ遷移");
			auto* sceneManager = core::base::ServiceLocator::get<game::scene::SceneManager>();
			sceneManager->changeScene(game::scene::SceneType::Result);
			return;
		}
		m_systemManager.update(deltaTime);
		auto& transform = m_componentManager.get<game::component::TransformComponent>(m_playerId);
		m_camera.update(transform.m_position, core::Vector3(CAMERA_OFFSET_X, CAMERA_OFFSET_Y, CAMERA_OFFSET_Z));

		// フレーム最後に入力状態を更新
		m_inputProvider.updatePreviousState();
	}

	void InGame::draw()
	{
		// コンポーネントの取得
		auto& transform = m_componentManager.get<game::component::TransformComponent>(m_playerId);
		// auto& anim = m_componentManager.get<game::component::AnimationComponent<game::constant::PlayerAnimationState>>(m_playerId);
		auto& groundRender = m_componentManager.get<game::component::RenderComponent>(m_groundId);
		auto& render = m_componentManager.get<game::component::RenderComponent>(m_playerId);
		auto& groundTransform = m_componentManager.get<game::component::TransformComponent>(m_groundId);

		// モデルの描画
		if (render.m_isVisible)
			m_renderer.drawModel(render.m_modelHandle, transform.m_position, transform.m_rotation, transform.m_scale);
		m_renderer.drawModel(groundRender.m_modelHandle, groundTransform.m_position, groundTransform.m_rotation, groundTransform.m_scale);

		// 敵の描画
		auto& enemyRenderer = m_componentManager.get<component::RenderComponent>(m_enemyId);
		auto& enemyTransform = m_componentManager.get<component::TransformComponent>(m_enemyId);
		if (enemyRenderer.m_isVisible)
			m_renderer.drawModel(enemyRenderer.m_modelHandle, enemyTransform.m_position, enemyTransform.m_rotation, enemyTransform.m_scale);

		// デバッグ: コライダーを可視化
		auto& playerCollider = m_componentManager.get<game::component::ColliderComponent>(m_playerId);
		auto& groundCollider = m_componentManager.get<game::component::ColliderComponent>(m_groundId);

		core::Vector3 playerColliderCenter = transform.m_position + playerCollider.m_offset;
		core::Vector3 groundColliderCenter = groundTransform.m_position + groundCollider.m_offset;

		m_renderer.drawCollider(playerColliderCenter, playerCollider.m_size, core::utility::Color::GREEN);
		m_renderer.drawCollider(groundColliderCenter, groundCollider.m_size, core::utility::Color::BLUE);
	}
}
