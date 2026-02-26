#include "InGameScene.h"
#include "game/ecs/system/InputSystem.h"
#include "game/ecs/system/MoveSystem.h"
#include "game/ecs/system/PhysicsSystem.h"
#include "game/ecs/component/TransformComponent.h"
#include "game/ObjectFactory.h"
#include "core/utility/LogUtil.h"
#include "core/ServiceLocator.h"
#include "engine/Camera.h"

namespace engine::scene
{
	InGameScene::InGameScene()
		: m_objectFactory(m_entityManager, m_componentManager)
	{
		m_objectFactory.init();

		m_systemManager.registerSystem<game::ecs::system::InputSystem>(m_componentManager, m_objectFactory.getPlayer().getId());
		m_systemManager.registerSystem<game::ecs::system::MoveSystem>(m_componentManager, m_objectFactory.getPlayer().getId(), PLAYER_MOVE_SPEED);
		m_systemManager.registerSystem<game::ecs::system::PhysicsSystem>(m_componentManager, m_objectFactory.getPlayer().getId());
	}

	void InGameScene::update(float deltaTime)
	{
		m_systemManager.update(deltaTime);

		auto& transform = m_componentManager.get<game::ecs::component::TransformComponent>(m_objectFactory.getPlayer().getId());
		// カメラ更新
		core::ServiceLocator::get<engine::Camera>()->update(transform.m_position, core::Vector3(0.0f, 800.0f, -600.0f));

		// 現在位置を出力する(デバック)

		core::utility::LogUtil::log("x: %8.2f  y: %8.2f  z: %8.2f\n",
			transform.m_position.x,
			transform.m_position.y,
			transform.m_position.z);
	}
}