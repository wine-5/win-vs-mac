#include "PhysicsSystem.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/movement/InputComponent.h"
#include "game/component/combat/ProjectileComponent.h"

namespace game::system::movement
{
	PhysicsSystem::PhysicsSystem(core::ecs::ComponentManager& componentManager, float jumpForce)
	    : m_componentManager{ componentManager }
	    , m_jumpForce{ jumpForce }
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

			// 入力による速度と、外から加わる速度（坂の滑り等）を合算して動かす
			const core::Vector3 total{ velocity.m_velocity + velocity.m_externalVelocity };
			transform.m_position.x += total.x * deltaTime;
			transform.m_position.y += total.y * deltaTime;
			transform.m_position.z += total.z * deltaTime;
		}
	}
} // namespace game::system::movement