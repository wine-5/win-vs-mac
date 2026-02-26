#include "InGameScene.h"
#include "game/ecs/system/InputSystem.h"
#include "game/ecs/system/MoveSystem.h"
#include "game/ecs/system/PhysicsSystem.h"
#include "game/ecs/component/TransformComponent.h"
#include "game/ObjectFactory.h"
#include "utility/LogUtil.h"

namespace game::scene
{
	InGameScene::InGameScene()
		: m_objectFactory(m_entityManager, m_componentManager)
	{
		m_objectFactory.init();

		m_systemManager.registerSystem<ecs::system::InputSystem>(m_componentManager,m_objectFactory.getPlayer().getId());
		m_systemManager.registerSystem<ecs::system::MoveSystem>(m_componentManager, m_objectFactory.getPlayer().getId(), PLAYER_MOVE_SPEED);
		m_systemManager.registerSystem<ecs::system::PhysicsSystem>(m_componentManager, m_objectFactory.getPlayer().getId());
	}

	void InGameScene::update(float deltaTime)
	{
		m_systemManager.update(deltaTime);

		// 現在位置を出力する（デバック）
		auto& transform = m_componentManager.get<ecs::component::TransformComponent>(m_objectFactory.getPlayer().getId());

		utility::LogUtil::log("x: %8.2f  y: %8.2f  z: %8.2f\n",
			transform.m_position.x,
			transform.m_position.y,
			transform.m_position.z);
	}
}