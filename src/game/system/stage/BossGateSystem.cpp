#include "BossGateSystem.h"
#include "game/component/stage/BossGateComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/event/InGameEvents.h"
#include <algorithm>

namespace
{
	// 扉が閉じきるまでの時間（秒）
	constexpr float CLOSE_DURATION{ 1.2f };
} // namespace

namespace game::system::stage
{
	BossGateSystem::BossGateSystem(core::ecs::ComponentManager& componentManager,
	    core::base::EventBus& eventBus)
	    : m_componentManager{ componentManager }
	{
		m_subscriptions.push_back(eventBus.subscribe<event::BossAppearedEvent>(
		    [this](const event::BossAppearedEvent&)
		    { m_isTriggered = true; }));
	}

	void BossGateSystem::update(float deltaTime)
	{
		if (!m_isTriggered)
			return;

		const auto gates{ m_componentManager.getAllEntities<component::stage::BossGateComponent>() };
		bool anyMoving{ false };

		for (const auto gateId : gates)
		{
			auto& gate{ m_componentManager.get<component::stage::BossGateComponent>(gateId) };
			gate.m_isClosing = true;

			if (gate.m_progress >= 1.0f)
				continue;

			gate.m_progress = std::min(gate.m_progress + deltaTime / CLOSE_DURATION, 1.0f);
			anyMoving = true;

			// 加速しながら上がって閉じきる瞬間が最も速い（シャッターが叩きつけられる感じ）
			const float eased{ gate.m_progress * gate.m_progress };

			auto& transform{ m_componentManager.get<component::movement::TransformComponent>(gateId) };
			transform.m_position.y = gate.m_openY + (gate.m_closedY - gate.m_openY) * eased;
		}

		// 全ての扉が閉じきったら、以降は毎フレーム走査しない
		if (!anyMoving)
			m_isTriggered = false;
	}
} // namespace game::system::stage
