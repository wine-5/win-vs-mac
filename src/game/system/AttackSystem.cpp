#include "AttackSystem.h"
#include "game/component/AttackComponent.h"
#include "game/component/TransformComponent.h"
#include "game/component/HealthComponent.h"
#include "game/attack/DamageChain.h"
#include "game/attack/BaseAttackHandler.h"
#include "game/attack/DefenseHandler.h"
#include "game/event/InGameEvents.h"
#include <cmath>

namespace game::system
{
	AttackSystem::AttackSystem(core::ecs::ComponentManager& componentManager, EventBus& eventBus)
		: m_componentManager{ componentManager }
		, m_eventBus{ eventBus }
	{
	}

	void AttackSystem::update(float deltaTime)
	{
		auto attakers{ m_componentManager.getAllEntities<component::AttackComponent>() };
		
		for (auto attakerId : attakers)
		{
			auto& attack{ m_componentManager.get<component::AttackComponent>(attakerId) };
			
			// クールダウンを更新
			if (attack.m_currentCooldown > 0.0f)
			{
				attack.m_currentCooldown -= deltaTime;
				continue;
			}
			
			auto& attakerTransform{ m_componentManager.get<component::TransformComponent>(attakerId) };
			auto targets{ m_componentManager.getAllEntities<component::HealthComponent>() };
			
			for (auto targetId : targets)
			{
				if (targetId == attakerId)
					continue;

				//auto& target
			}

		}
	}
}