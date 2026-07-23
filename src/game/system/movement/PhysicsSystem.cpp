#include "PhysicsSystem.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/InputComponent.h"
#include "game/component/combat/ProjectileComponent.h"

namespace game::system::movement
{
	PhysicsSystem::PhysicsSystem(core::ecs::ComponentManager& componentManager)
		: m_componentManager{ componentManager }
	{
	}

	void PhysicsSystem::update(float deltaTime)
	{
		auto entities{m_componentManager.getAllEntities<component::VelocityComponent>()};

		for (auto& entityId : entities)
		{
			auto& transform = m_componentManager.get<component::TransformComponent>(entityId);
			auto& velocity = m_componentManager.get<component::VelocityComponent>(entityId);

			// 弾は直進させたいので、ジャンプ・重力の対象外にする
			if (!m_componentManager.has<component::combat::ProjectileComponent>(entityId))
			{
				if (m_componentManager.has<component::InputComponent>(entityId))
				{
					auto& input = m_componentManager.get<component::InputComponent>(entityId);
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