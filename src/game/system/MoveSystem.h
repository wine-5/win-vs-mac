#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

namespace game::system
{
	/**
	 * @brief 入力を元に速度を計算するSystem
	 */
	class MoveSystem : public core::ecs::ISystem
	{
	public:
		/**
		 * @brief MoveSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param entityId 対象EntityのID
		 * @param moveSpeed 移動速度
		 */
		MoveSystem(core::ecs::ComponentManager& componentManager, core::ecs::EntityId entityId, float moveSpeed);
		
		/**
		 * @brief 入力を元に速度を計算する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	private:
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_entityId{};
		float m_moveSpeed{0.0f};
	};
} // namespace game::system