#include "PhysicsSystem.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/movement/InputComponent.h"
#include "game/component/combat/ProjectileComponent.h"

namespace game::system::movement
{
	PhysicsSystem::PhysicsSystem(core::ecs::ComponentManager& componentManager)
		: m_componentManager{ componentManager }
	{
	}

	void PhysicsSystem::update(float deltaTime)
	{
		auto entities{ m_componentManager.getAllEntities<component::movement::VelocityComponent>() };

		for (auto& entityId : entities)
		{
			auto& transform = m_componentManager.get<component::movement::TransformComponent>(entityId);
			auto& velocity = m_componentManager.get<component::movement::VelocityComponent>(entityId);

			// 弾は直進させたいので、ジャンプ・重力の対象外にする
			if (!m_componentManager.has<component::combat::ProjectileComponent>(entityId))
			{
				if (m_componentManager.has<component::movement::InputComponent>(entityId))
				{
					auto& input = m_componentManager.get<component::movement::InputComponent>(entityId);
					// ジャンプ処理
					if (input.m_jumpPressed)
						velocity.m_velocity.y = m_jumpForce;
				}

				// 重力
				velocity.m_velocity.y += m_gravity * deltaTime;
				// 速度を制限（トンネリング防止）
				if (velocity.m_velocity.y < m_maxFallSpeed)
					velocity.m_velocity.y = m_maxFallSpeed;
			}

			transform.m_position.x += velocity.m_velocity.x * deltaTime;
			transform.m_position.y += velocity.m_velocity.y * deltaTime;
			transform.m_position.z += velocity.m_velocity.z * deltaTime;
		}
	}
} // namespace game::system::movement