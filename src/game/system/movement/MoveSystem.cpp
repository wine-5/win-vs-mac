#include "MoveSystem.h"
#include "game/component/movement/InputComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/AnimationComponent.h"
#include "game/component/CameraComponent.h"
#include <cmath>

namespace game::system::movement
{
	MoveSystem::MoveSystem(core::ecs::ComponentManager& componentManager, core::ecs::EntityId entityId, float moveSpeed, float dashMultiplier)
	    : m_componentManager{ componentManager }, m_entityId{ entityId }, m_moveSpeed{ moveSpeed }, m_dashMultiplier{ dashMultiplier }
	{
	}

	void MoveSystem::update(float deltaTime)
	{
		auto& input = m_componentManager.get<component::movement::InputComponent>(m_entityId);
		auto& velocity = m_componentManager.get<component::movement::VelocityComponent>(m_entityId);
		auto& transform = m_componentManager.get<component::movement::TransformComponent>(m_entityId);

		// カメラのyawを基準に、入力をカメラ相対のワールド方向へ変換する
		float cameraYaw{ 0.0f };
		if (m_componentManager.has<component::CameraComponent>(m_entityId))
			cameraYaw = m_componentManager.get<component::CameraComponent>(m_entityId).m_yaw;

		const float sinYaw{ std::sin(cameraYaw) };
		const float cosYaw{ std::cos(cameraYaw) };

		// 前方向(W/S)=カメラの向き、右方向(A/D)=その直交
		const float forwardX{ sinYaw };
		const float forwardZ{ cosYaw };
		const float rightX{ cosYaw };
		const float rightZ{ -sinYaw };

		float worldX{ input.m_moveX * rightX + input.m_moveZ * forwardX };
		float worldZ{ input.m_moveX * rightZ + input.m_moveZ * forwardZ };

		// 斜め入力で速くならないよう正規化する
		const float length{ std::sqrt(worldX * worldX + worldZ * worldZ) };
		if (length > 0.0f)
		{
			worldX /= length;
			worldZ /= length;
		}

		const float speed{ m_moveSpeed * (input.m_dashPressed ? m_dashMultiplier : 1.0f) };
		velocity.m_velocity.x = worldX * speed;
		velocity.m_velocity.z = worldZ * speed;

		// 移動入力があるときだけ向きを更新する
		const bool isMoving{ length > 0.0f };
		if (isMoving)
			transform.m_rotation.y = atan2f(-worldX, -worldZ);

		// 移動状態に応じたアニメーションを要求する
		if (m_componentManager.has<component::AnimationComponent>(m_entityId))
		{
			auto& anim = m_componentManager.get<component::AnimationComponent>(m_entityId);
			anim.m_requested = isMoving
				? constant::AnimationState::Walk
				: constant::AnimationState::Idle;
		}
	}
} // namespace game::system::movement
