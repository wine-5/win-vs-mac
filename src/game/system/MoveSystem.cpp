#include "MoveSystem.h"
#include "game/component/InputComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/TransformComponent.h"
#include <cmath>

namespace game::system
{
	MoveSystem::MoveSystem(core::ecs::ComponentManager& componentManager, core::ecs::EntityId entityId, float moveSpeed)
		: m_componentManager(componentManager)
		, m_entityId(entityId)
		, m_moveSpeed(moveSpeed)
	{
	}

	void MoveSystem::update(float deltaTime)
	{
		auto& input = m_componentManager.get<component::InputComponent>(m_entityId);
		auto& velocity = m_componentManager.get<component::VelocityComponent>(m_entityId);
		auto& transform = m_componentManager.get<component::TransformComponent>(m_entityId);

		velocity.m_velocity.x = input.m_moveX * m_moveSpeed;
		velocity.m_velocity.z = input.m_moveZ * m_moveSpeed;

		// 移動入力があるときだけ向きを更新する
		if (input.m_moveX != 0.0f || input.m_moveZ != 0.0f)
			transform.m_rotation.y = atan2f(-input.m_moveX, -input.m_moveZ);
	}
}