#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

namespace game::system
{
	/**
	 * @brief 速度を元に位置を更新・重力・ジャンプを処理するSystem
	 */
	class PhysicsSystem : public core::ecs::ISystem
	{
	public:
		PhysicsSystem(core::ecs::ComponentManager& componentManager, core::ecs::EntityId playerId);
		void update(float deltaTime) override;

	private:
		static constexpr float DEFAULT_GRAVITY = -98.0f;
		static constexpr float DEFAULT_JUMP_FORCE = 50.0f;
		static constexpr float DEFAULT_MAX_FALL_SPEED = -200.0f;

		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_playerId;
		float m_gravity = DEFAULT_GRAVITY;
		float m_jumpForce = DEFAULT_JUMP_FORCE;
		float m_maxFallSpeed = DEFAULT_MAX_FALL_SPEED;
	};
}
