#pragma once
#include "game/ecs/ISystem.h"
#include "game/ecs/ComponentManager.h"
#include "game/ecs/Entity.h"

namespace game::ecs::system
{
	/**
	 * @brief キーボード入力をInputComponentに反映するSystem
	 */
	class InputSystem : public ISystem
	{
	public:
		InputSystem(ComponentManager& componentManager, EntityId playerId);
		void update(float deltaTime) override;

	private:
		ComponentManager& m_componentManager;
		EntityId m_playerId;
	};
}
