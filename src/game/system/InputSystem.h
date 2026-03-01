#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

namespace game::system
{
	/**
	 * @brief キーボード入力をInputComponentに反映するSystem
	 */
	class InputSystem : public core::ecs::ISystem
	{
	public:
		static constexpr float INPUT_NEUTRAL = 0.0f;
		static constexpr float INPUT_POSITIVE = 1.0f;
		static constexpr float INPUT_NEGATIVE = -1.0f;

		InputSystem(core::ecs::ComponentManager& componentManager, core::ecs::EntityId playerId);
		void update(float deltaTime) override;

	private:
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_playerId;
	};
}
