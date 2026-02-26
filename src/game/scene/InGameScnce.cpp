#include "InGameScene.h"
#include "game/ecs/system/InputSystem.h"
#include "game/ecs/system/MoveSystem.h"
#include "game/ecs/system/PhysicsSystem.h"
#include <DxLib.h>
#include "game/ecs/component/TransformComponent.h"

namespace game::scene
{
	InGameScene::InGameScene()
		: m_player(m_entityManager, m_componentManager)
	{
		m_systemManager.registerSystem<ecs::system::InputSystem>(m_componentManager,m_player.getId());
		m_systemManager.registerSystem<ecs::system::MoveSystem>(m_componentManager, m_player.getId(), 5.0f);
		m_systemManager.registerSystem<ecs::system::PhysicsSystem>(m_componentManager, m_player.getId());
	}

	void InGameScene::update(float deltaTime)
	{
		m_systemManager.update(deltaTime);

		// 現在位置を出力する（デバック）
		auto& transform = m_componentManager.get<ecs::component::TransformComponent>(m_player.getId());
		clsDx();
		printfDx("x: %8.2f  y: %8.2f  z: %8.2f\n",
			transform.m_position.x,
			transform.m_position.y,
			transform.m_position.z);
	}
}