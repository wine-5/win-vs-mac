#include "InGameScene.h"
#include "game/system/InputSystem.h"
#include "game/system/MoveSystem.h"
#include "game/system/PhysicsSystem.h"
#include "game/component/TransformComponent.h"
#include "game/ObjectFactory.h"
#include "game/actor/Player.h"
#include "game/component/RenderComponent.h"
#include "core/interface/ILogger.h"
#include "game/system/AnimationSystem.h"
#include "game/system/CollisionSystem.h"

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
		, m_objectFactory(m_entityManager, m_componentManager, m_resourceManager)
	{
		m_objectFactory.init(m_resourceManager.loadModel(m_playerData.getModelPath()), m_playerData);

		// Ground生成
		int groundModelHandle = m_resourceManager.loadModel("assets/model/Ground.mv1");
		m_ground = std::make_unique<stage::Ground>(
			m_entityManager,
			m_componentManager,
			groundModelHandle,
			core::Vector3(0.0f, -900.0f, 0.0f),
			core::Vector3(1.0f, 10.0f, 1.0f)
		);

		int idleAnimHandle = m_resourceManager.loadModel(m_playerData.getIdleAnimPath());
		int walkAnimHandle = m_resourceManager.loadModel(m_playerData.getWalkAnimPath());

		m_systemManager.registerSystem<game::system::InputSystem>(m_componentManager, m_objectFactory.getPlayer().getId(), m_inputProvider);
		m_systemManager.registerSystem<game::system::MoveSystem>(m_componentManager, m_objectFactory.getPlayer().getId(), m_playerData.getMoveSpeed());
		m_systemManager.registerSystem<game::system::PhysicsSystem>(m_componentManager, m_objectFactory.getPlayer().getId());
		m_systemManager.registerSystem<game::system::AnimationSystem>(m_componentManager, m_objectFactory.getPlayer().getId(), m_animator,idleAnimHandle, walkAnimHandle);

		// System登録の後に追加
		auto* collisionSystem = m_systemManager.registerSystem<game::system::CollisionSystem>(m_componentManager);
		collisionSystem->addEntity(m_objectFactory.getPlayer().getId());
		collisionSystem->addEntity(m_ground->getId());
	}

	void InGameScene::update(float deltaTime)
	{
		m_systemManager.update(deltaTime);

		auto& transform = m_componentManager.get<game::component::TransformComponent>(m_objectFactory.getPlayer().getId());
		auto& render = m_componentManager.get<game::component::RenderComponent>(m_objectFactory.getPlayer().getId());
		auto& anim = m_componentManager.get<game::component::AnimationComponent<game::constant::PlayerAnimationState>>(m_objectFactory.getPlayer().getId());
		auto& groundRender = m_componentManager.get<game::component::RenderComponent>(m_ground->getId());
		auto& groundTransform = m_componentManager.get<game::component::TransformComponent>(m_ground->getId());

		m_camera.update(transform.m_position, core::Vector3(CAMERA_OFFSET_X, CAMERA_OFFSET_Y, CAMERA_OFFSET_Z));
		m_renderer.drawModel(render.m_modelHandle, transform.m_position,transform.m_rotation);
		m_renderer.drawModel(groundRender.m_modelHandle, groundTransform.m_position, groundTransform.m_rotation);

		/*LOG("animIndex: %d  animTime: %8.2f  animTotalTime: %8.2f\n",
			anim.m_animIndex,
			anim.m_animTime,
			anim.m_animTotalTime);*/

		LOG("x: %8.2f  y: %8.2f  z: %8.2f\n",
			transform.m_position.x,
			transform.m_position.y,
			transform.m_position.z);
	}
}