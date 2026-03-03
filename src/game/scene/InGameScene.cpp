#include "InGameScene.h"
#include "game/system/InputSystem.h"
#include "game/system/MoveSystem.h"
#include "game/system/PhysicsSystem.h"
#include "game/component/TransformComponent.h"
#include "game/ObjectFactory.h"
#include "game/actor/Player.h"
#include "game/component/RenderComponent.h"
#include "core/interface/ILogger.h"

namespace game::scene
{
	InGameScene::InGameScene(core::iface::ICamera& camera,
		core::iface::IRenderer& renderer,
		core::iface::IResourceManager& resourceManager,
		core::iface::IInputProvider& inputProvider)
		: m_camera(camera)
		, m_renderer(renderer)
		, m_resourceManager(resourceManager)
		, m_inputProvider(inputProvider)
		, m_objectFactory(m_entityManager, m_componentManager, m_resourceManager)
	{
		m_objectFactory.init();

		m_systemManager.registerSystem<game::system::InputSystem>(m_componentManager, m_objectFactory.getPlayer().getId(), m_inputProvider);
		m_systemManager.registerSystem<game::system::MoveSystem>(m_componentManager, m_objectFactory.getPlayer().getId(), game::actor::PLAYER_MOVE_SPEED);
		m_systemManager.registerSystem<game::system::PhysicsSystem>(m_componentManager, m_objectFactory.getPlayer().getId());
	}

	void InGameScene::update(float deltaTime)
	{
		m_systemManager.update(deltaTime);

		auto& transform = m_componentManager.get<game::component::TransformComponent>(m_objectFactory.getPlayer().getId());
		auto& render = m_componentManager.get<game::component::RenderComponent>(m_objectFactory.getPlayer().getId());

		m_camera.update(transform.m_position, core::Vector3(CAMERA_OFFSET_X, CAMERA_OFFSET_Y, CAMERA_OFFSET_Z));
		m_renderer.drawModel(render.m_modelHandle, transform.m_position,transform.m_rotation);

		LOG("x: %8.2f  y: %8.2f  z: %8.2f\n",
			transform.m_position.x,
			transform.m_position.y,
			transform.m_position.z);
	}
}