#include "InGameScene.h"
#include "game/system/InputSystem.h"
#include "game/system/MoveSystem.h"
#include "game/system/PhysicsSystem.h"
#include "game/component/TransformComponent.h"
#include "game/actor/Player.h"
#include "game/component/RenderComponent.h"
#include "core/interface/ILogger.h"
#include "game/system/AnimationSystem.h"
#include "game/system/CollisionSystem.h"
#include "game/component/ColliderComponent.h"
#include "game/constant/ModelId.h"
#include <cassert>

namespace game::scene
{
	InGameScene::InGameScene(core::iface::ICamera& camera,
		core::iface::IRenderer& renderer,
		core::iface::IAnimator& animator,
		core::iface::IResourceManager& resourceManager,
		core::iface::IInputProvider& inputProvider)
		: m_camera(camera)
		, m_renderer(renderer)
		, m_animator(animator)
		, m_resourceManager(resourceManager)
		, m_inputProvider(inputProvider)
		, m_factoryManager(m_entityManager, m_componentManager, m_resourceManager)
		, m_playerData(game::data::PlayerData::fromMetadata(
			m_resourceManager.getMetadata(constant::model_id::PLAYER).value()))
	{
		loadResources();
		spawnEntities();
		setupSystems();
	}

	void InGameScene::loadResources()
	{
		// 先にモデルをロードして自動計算を実行
		int playerModelHandle = m_resourceManager.loadModelById(constant::model_id::PLAYER);

		// モデルロード後に再度メタデータを取得してPlayerDataを更新
		auto playerMeta = m_resourceManager.getMetadata(constant::model_id::PLAYER);
		assert(playerMeta.has_value() && "Playerのメタデータが見つかりません");
		m_playerData = game::data::PlayerData::fromMetadata(playerMeta.value());

	}
	void InGameScene::spawnEntities()
	{
		
		m_factoryManager.getPlayerFactory().create(playerModelHandle, m_playerData);

		// Ground生成（JSON駆動）
		int groundModelHandle = m_resourceManager.loadModelById(constant::model_id::GROUND);
		auto groundMeta = m_resourceManager.getMetadata(constant::model_id::GROUND);
		assert(groundMeta.has_value() && "Groundのメタデータが見つかりません");

		game::data::GroundData groundData = game::data::GroundData::fromMetadata(groundMeta.value());
		m_groundId = m_factoryManager.getGroundFactory().create(
			groundModelHandle,
			groundData
		);

	}
	void InGameScene::setupSystems()
	{
		// アニメーションハンドルをロード
		//int idleAnimHandle = m_resourceManager.loadModelById(m_playerData.getIdleAnimPath());
		//int walkAnimHandle = m_resourceManager.loadModelById(m_playerData.getWalkAnimPath());

		// システム登録
		m_systemManager.registerSystem<game::system::InputSystem>(m_componentManager, m_factoryManager.getPlayerFactory().getPlayer().getId(), m_inputProvider);
		m_systemManager.registerSystem<game::system::MoveSystem>(m_componentManager, m_factoryManager.getPlayerFactory().getPlayer().getId(), m_playerData.getMoveSpeed());
		m_systemManager.registerSystem<game::system::PhysicsSystem>(m_componentManager, m_factoryManager.getPlayerFactory().getPlayer().getId());
		//m_systemManager.registerSystem<game::system::AnimationSystem>(
		//	m_componentManager,
		//	m_factoryManager.getPlayerFactory().getPlayer().getId(),
		//	m_animator,
		//	idleAnimHandle,
		//	walkAnimHandle);

		auto* collisionSystem = m_systemManager.registerSystem<game::system::CollisionSystem>(m_componentManager);
		collisionSystem->addEntity(m_factoryManager.getPlayerFactory().getPlayer().getId());
		collisionSystem->addEntity(m_groundId);
	}

	void InGameScene::update(float deltaTime)
	{
		m_systemManager.update(deltaTime);

		auto& transform = m_componentManager.get<game::component::TransformComponent>(m_factoryManager.getPlayerFactory().getPlayer().getId());
		auto& render = m_componentManager.get<game::component::RenderComponent>(m_factoryManager.getPlayerFactory().getPlayer().getId());
		auto& anim = m_componentManager.get<game::component::AnimationComponent<game::constant::PlayerAnimationState>>(m_factoryManager.getPlayerFactory().getPlayer().getId());
		auto& groundRender = m_componentManager.get<game::component::RenderComponent>(m_groundId);
		auto& groundTransform = m_componentManager.get<game::component::TransformComponent>(m_groundId);

		m_camera.update(transform.m_position, core::Vector3(CAMERA_OFFSET_X, CAMERA_OFFSET_Y, CAMERA_OFFSET_Z));
		m_renderer.drawModel(render.m_modelHandle, transform.m_position,transform.m_rotation,transform.m_scale);
		m_renderer.drawModel(groundRender.m_modelHandle, groundTransform.m_position, groundTransform.m_rotation, groundTransform.m_scale);

		// デバッグ: コライダーを可視化
		auto& playerCollider = m_componentManager.get<game::component::ColliderComponent>(m_factoryManager.getPlayerFactory().getPlayer().getId());
		auto& groundCollider = m_componentManager.get<game::component::ColliderComponent>(m_groundId);
		
		core::Vector3 playerColliderCenter = transform.m_position + playerCollider.m_offset;
		core::Vector3 groundColliderCenter = groundTransform.m_position + groundCollider.m_offset;
		
		m_renderer.drawCollider(playerColliderCenter, playerCollider.m_size, 0x00FF00); // 緑: Player
		m_renderer.drawCollider(groundColliderCenter, groundCollider.m_size, 0x0000FF); // 青: Ground
	}
}