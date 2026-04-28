#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/EventBus.h"

namespace game::system
{
	class AttackSystem : public core::ecs::ISystem
	{
	public:
		AttackSystem(core::ecs::ComponentManager& componentManager, EventBus& eventBus);

		void update(float deltaTime) override;

	private:
		core::ecs::ComponentManager& m_componentManager;
		EventBus& m_eventBus;
	};
}