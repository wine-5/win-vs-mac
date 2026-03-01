#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

namespace game::ecs::system
{
	/**
	 * @brief 速度を元に位置を更新・重力・ジャンプを処理するSystem
	 */
	class PhysicsSystem : public ISystem
	{
	public:
		PhysicsSystem(ComponentManager& componentManager, EntityId playerId);
		void update(float deltaTime) override;

	private:
		static constexpr float DEFAULT_GRAVITY = -9.8f;
		static constexpr float DEFAULT_JUMP_FORCE = 5.0f;
		static constexpr float DEFAULT_GROUND_Y = 0.0f;

		bool isGrounded(float positionY) const;

		ComponentManager& m_componentManager;
		EntityId m_playerId;

		float m_gravity = DEFAULT_GRAVITY;
		float m_jumpForce = DEFAULT_JUMP_FORCE;
		float m_groundY = DEFAULT_GROUND_Y; // 地面の座標（今後は床の大きさに動的に設定できるようにするが一時的にテストするために)
	};
}
