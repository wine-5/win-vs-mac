#pragma once
#include "game/ecs/ISystem.h"
#include "game/ecs/ComponentManager.h"
#include "game/ecs/Entity.h"

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
		bool isGrounded(float positionY) const;

		ComponentManager& m_componentManager;
		EntityId m_playerId;

		float m_gravity = -9.8f;
		float m_jumpForce = 5.0f;
		float m_groundY = 0.0f; // 地面の座標（今後は床の大きさに動的に設定できるようにするが一時的にテストするために)
	};
}
