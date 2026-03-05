#include "PhysicsSystem.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/InputComponent.h"

namespace game::system
{
	PhysicsSystem::PhysicsSystem(core::ecs::ComponentManager& componentManager, core::ecs::EntityId playerId)
		: m_componentManager(componentManager)
		, m_playerId(playerId)
	{
	}

	void PhysicsSystem::update(float deltaTime)
	{
		auto& transform = m_componentManager.get<component::TransformComponent>(m_playerId);
		auto& velocity = m_componentManager.get<component::VelocityComponent>(m_playerId);
		auto& input = m_componentManager.get<component::InputComponent>(m_playerId);

		// ジャンプ処理
		if (input.m_jumpPressed)
			velocity.m_velocity.y = m_jumpForce;

		// 重力
		velocity.m_velocity.y += m_gravity * deltaTime;

		transform.m_position.x += velocity.m_velocity.x * deltaTime;
		transform.m_position.y += velocity.m_velocity.y * deltaTime;
		transform.m_position.z += velocity.m_velocity.z * deltaTime;
	}
}