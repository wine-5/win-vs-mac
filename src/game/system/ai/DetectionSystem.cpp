#include "DetectionSystem.h"
#include "game/component/ai/AIComponent.h"
#include "game/component/TransformComponent.h"
#include "game/event/InGameEvents.h"
#include <cmath>

namespace game::system::ai
{
	DetectionSystem::DetectionSystem(core::ecs::ComponentManager& componentManager, core::base::EventBus& eventBus)
	    : m_componentManager{ componentManager }
	    , m_eventBus{ eventBus }
	{
	}

	void DetectionSystem::update(float /*deltaTime*/)
	{
		// AIComponentを持つ＝敵。種類を問わず一律に索敵の切り替わりを見る
		auto entities{ m_componentManager.getAllEntities<component::ai::AIComponent>() };

		for (auto entityId : entities)
		{
			auto& ai{ m_componentManager.get<component::ai::AIComponent>(entityId) };

			// 死亡・停止中の敵は発見判定しない
			if (!ai.m_isActive)
				continue;

			bool aware{ false };
			if (ai.m_targetEntity.getId() != 0 &&
			    m_componentManager.has<component::TransformComponent>(entityId) &&
			    m_componentManager.has<component::TransformComponent>(ai.m_targetEntity.getId()))
			{
				const auto& transform{ m_componentManager.get<component::TransformComponent>(entityId) };
				const auto& targetTransform{ m_componentManager.get<component::TransformComponent>(ai.m_targetEntity.getId()) };

				const float dx{ targetTransform.m_position.x - transform.m_position.x };
				const float dz{ targetTransform.m_position.z - transform.m_position.z };
				const float distance{ std::sqrt(dx * dx + dz * dz) };
				aware = distance <= ai.m_detectionRange;
			}

			// 未索敵→索敵に切り替わった瞬間だけ発見イベントを発行する
			if (aware && !ai.m_wasAware)
				m_eventBus.publish(event::EnemyAlertedEvent{ entityId });

			ai.m_wasAware = aware;
		}
	}
} // namespace game::system::ai
