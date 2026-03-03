#include "InGameScene.h"
#include "game/system/InputSystem.h"
#include "game/system/MoveSystem.h"
#include "game/system/PhysicsSystem.h"
#include "game/component/TransformComponent.h"
#include "game/ObjectFactory.h"
#include "infrastructure/utility/LogUtil.h"
#include "infrastructure/Camera.h"
#include "infrastructure/Renderer.h"
#include "infrastructure/ResourceManager.h"
#include "game/actor/Player.h"

namespace game::scene
{
	InGameScene::InGameScene()
		: m_objectFactory(m_entityManager, m_componentManager, m_resourceManager)
	{
		m_objectFactory.init();

		m_systemManager.registerSystem<game::system::InputSystem>(m_componentManager, m_objectFactory.getPlayer().getId(), m_inputManager);
		m_systemManager.registerSystem<game::system::MoveSystem>(m_componentManager, m_objectFactory.getPlayer().getId(), game::actor::PLAYER_MOVE_SPEED);
		m_systemManager.registerSystem<game::system::PhysicsSystem>(m_componentManager, m_objectFactory.getPlayer().getId());
	}

	void InGameScene::update(float deltaTime)
	{
		m_systemManager.update(deltaTime);

		auto& transform = m_componentManager.get<game::component::TransformComponent>(m_objectFactory.getPlayer().getId());
		auto& render = m_componentManager.get<game::component::RenderComponent>(m_objectFactory.getPlayer().getId());

		m_camera.update(transform.m_position, core::Vector3(CAMERA_OFFSET_X, CAMERA_OFFSET_Y, CAMERA_OFFSET_Z));
		m_renderer.drawModel(render.m_modelHandle, transform.m_position);

		LOG("x: %8.2f  y: %8.2f  z: %8.2f\n",
			transform.m_position.x,
			transform.m_position.y,
			transform.m_position.z);
	}
}