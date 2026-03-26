#include "AISystem.h"
#include "game/component/AIComponent.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "core/Vector3.h"
#include <cmath>

namespace game::system
{
	AISystem::AISystem(core::ecs::ComponentManager& componentManaager)
		: m_componentManager(componentManaager)
	{
	}

	void AISystem::update(float deltaTime)
	{
		// AIComponentを持つ全エンティティを取得
		auto entities = m_componentManager.getAllEntities<component::AIComponent>();
		for (auto entityId : entities)
		{
			auto& ai = m_componentManager.get<component::AIComponent>(entityId);

			// AIが無効なら処理をスキップ
			if (!ai.m_isActive)
				continue;

			// 追跡対象が設定されていない場合はスキップ
			if(ai.m_targetEntity.getId() == 0)
			continue;

			auto& trasform = m_componentManager.get<component::AIComponent>(entityId);
			auto& targetTransform = m_componentManager.get<component::TransformComponent>(ai.m_targetEntity.getId());


		}
	}
}