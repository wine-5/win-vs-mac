#include "PhysicsSystem.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/InputComponent.h"

namespace game::system
{
	PhysicsSystem::PhysicsSystem(core::ecs::ComponentManager& componentManager, core::ecs::EntityId entityId)
		: m_componentManager{componentManager}
		, m_entityId{entityId}
	{
	}

	void PhysicsSystem::update(float deltaTime)
	{
		auto& transform = m_componentManager.get<component::TransformComponent>(m_entityId);
		auto& velocity = m_componentManager.get<component::VelocityComponent>(m_entityId);
		auto& input = m_componentManager.get<component::InputComponent>(m_entityId);

		// ジャンプ処理
		if (input.m_jumpPressed)
			velocity.m_velocity.y = m_jumpForce;

		// 重力
		velocity.m_velocity.y += m_gravity * deltaTime;

		// 速度を制限（トンネリング防止）
		if (velocity.m_velocity.y < m_maxFallSpeed)
			velocity.m_velocity.y = m_maxFallSpeed;

		transform.m_position.x += velocity.m_velocity.x * deltaTime;
		transform.m_position.y += velocity.m_velocity.y * deltaTime;
		transform.m_position.z += velocity.m_velocity.z * deltaTime;
	}
}