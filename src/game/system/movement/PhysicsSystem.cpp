#include "PhysicsSystem.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/movement/InputComponent.h"
#include "game/component/combat/ProjectileComponent.h"
#include "game/component/visual/AnimationComponent.h"

namespace game::system::movement
{
	PhysicsSystem::PhysicsSystem(core::ecs::ComponentManager& componentManager, GameManager& gameManager, float jumpForce, float gravity, float maxFallSpeed)
	    : m_componentManager{ componentManager }
	    , m_gameManager{ gameManager }
	    , m_gravity{ gravity }
	    , m_jumpForce{ jumpForce }
	    , m_maxFallSpeed{ maxFallSpeed }
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

					// ジャンプの可否判定。
					// 通常は「接地中に押した瞬間」だけ跳ぶ（押しっぱなしでの浮上・空中ジャンプを防ぐ）。
					// デバッグの連続ジャンプ有効時は毎フレーム跳べるようにして空中移動を許す。
					const bool jumpEdge{ input.m_jumpPressed && !m_prevJumpPressed };
					const bool canJump{ m_gameManager.isContinuousJumpEnabled()
						                    ? input.m_jumpPressed
						                    : (jumpEdge && velocity.m_isGrounded) };
					if (canJump)
					{
						velocity.m_velocity.y = m_jumpForce;

						// ジャンプアニメを要求する（MoveSystemの移動要求より後に走るため上書きできる。
						// Jumpは優先度が高く、着地＝再生完了までlocomotion要求に割り込まれない）
						if (m_componentManager.has<component::visual::AnimationComponent>(entityId))
							m_componentManager.get<component::visual::AnimationComponent>(entityId).m_requested =
							    constant::AnimationState::Jump;
					}
					m_prevJumpPressed = input.m_jumpPressed;
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