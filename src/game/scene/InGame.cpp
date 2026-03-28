#include "InGame.h"

/* core層 */
#include "core/interface/ILogger.h"
#include "core/utility/Color.h"
#include "core/ServiceLocator.h"

/* game層 */
#include "game/factory/FactoryInitializer.h"
#include "game/system/InputSystem.h"
#include "game/system/MoveSystem.h"
#include "game/system/PhysicsSystem.h"
#include "game/component/TransformComponent.h"
#include "game/actor/Player.h"
#include "game/component/RenderComponent.h"
#include "game/system/AnimationSystem.h"
#include "game/system/CollisionSystem.h"
#include "game/component/ColliderComponent.h"
#include "game/constant/ModelId.h"
#include "game/scene/SceneManager.h"
#include "game/scene/SceneType.h"

/* 標準のインクルード */
#include <cassert>
#include <stdexcept>

namespace game::scene
{
	InGame::InGame(core::iface::ICamera& camera,
		core::iface::IRenderer& renderer,
		core::iface::IAnimator& animator,
		core::iface::IResourceManager& resourceManager,
		core::iface::IInputProvider& inputProvider)
		: m_camera{camera}
		, m_renderer{renderer}
		, m_animator{animator}
		, m_resourceManager{resourceManager}
		, m_inputProvider{inputProvider}
		, m_factoryManager{m_entityManager, m_componentManager, m_resourceManager}
		, m_playerData{game::data::PlayerData::fromMetadata(
			m_resourceManager.getMetadata(constant::model_id::PLAYER).value())}
	{
		loadResources();
		spawnEntities();
		setupSystems();
	}

	void InGame::loadResources()
	{
		// 先にモデルをロードして自動計算を実行
		m_resourceManager.loadModelById(constant::model_id::PLAYER);

		// モデルロード後に再度メタデータを取得してPlayerDataを更新
		auto playerMeta = m_resourceManager.getMetadata(constant::model_id::PLAYER);
		if (!playerMeta.has_value()) {
			LOG("ERROR: Playerのメタデータが見つかりません");
			throw std::runtime_error("Playerのメタデータの読み込みに失敗しました");
		}
		assert(playerMeta.has_value() && "Playerのメタデータが見つかりません");
		m_playerData = game::data::PlayerData::fromMetadata(playerMeta.value());

	}

	void InGame::spawnEntities()
	{
		
		game::factory::FactoryInitializer initializer(m_factoryManager, m_resourceManager);
		initializer.initializePlayer(m_playerData);
		m_playerId = m_factoryManager.getPlayerFactory().getPlayer().getId();
		m_groundId = initializer.initializeGround();

	}

	void InGame::setupSystems()
	{
		// アニメーションハンドルをロード
		//int idleAnimHandle = m_resourceManager.loadModelById(m_playerData.getIdleAnimPath());
		//int walkAnimHandle = m_resourceManager.loadModelById(m_playerData.getWalkAnimPath());

		// システム登録
		m_systemManager.registerSystem<game::system::InputSystem>(m_componentManager, m_playerId, m_inputProvider);
		m_systemManager.registerSystem<game::system::MoveSystem>(m_componentManager, m_playerId, m_playerData.getMoveSpeed());
		m_systemManager.registerSystem<game::system::PhysicsSystem>(m_componentManager);
		//m_systemManager.registerSystem<game::system::AnimationSystem>(
		//	m_componentManager,
		//	m_factoryManager.getPlayerFactory().getPlayer().getId(),
		//	m_animator,
		//	idleAnimHandle,
		//	walkAnimHandle);

		m_systemManager.registerSystem<game::system::CollisionSystem>(m_componentManager);
	}

	void InGame::update(float deltaTime)
	{
		// デバッグ用：Rキーでリザルト画面へ
		if (m_inputProvider.isKeyPressed(core::input::KeyCode::R))
		{
			LOG("INFO: Rキーでリザルト画面へ遷移");
			auto* sceneManager = core::ServiceLocator::get<game::scene::SceneManager>();
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
		auto& anim = m_componentManager.get<game::component::AnimationComponent<game::constant::PlayerAnimationState>>(m_playerId);
		auto& groundRender = m_componentManager.get<game::component::RenderComponent>(m_groundId);
		auto& render = m_componentManager.get<game::component::RenderComponent>(m_playerId);
		auto& groundTransform = m_componentManager.get<game::component::TransformComponent>(m_groundId);

		m_renderer.drawModel(render.m_modelHandle, transform.m_position,transform.m_rotation,transform.m_scale);
		m_renderer.drawModel(groundRender.m_modelHandle, groundTransform.m_position, groundTransform.m_rotation, groundTransform.m_scale);

		// デバッグ: コライダーを可視化
		auto& playerCollider = m_componentManager.get<game::component::ColliderComponent>(m_playerId);
		auto& groundCollider = m_componentManager.get<game::component::ColliderComponent>(m_groundId);
		
		core::Vector3 playerColliderCenter = transform.m_position + playerCollider.m_offset;
		core::Vector3 groundColliderCenter = groundTransform.m_position + groundCollider.m_offset;
		
		m_renderer.drawCollider(playerColliderCenter, playerCollider.m_size, core::utility::Color::GREEN);
		m_renderer.drawCollider(groundColliderCenter, groundCollider.m_size, core::utility::Color::BLUE);

	}
}