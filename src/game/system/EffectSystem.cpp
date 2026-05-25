#include "game/system/EffectSystem.h"
#include "core/base/ServiceLocator.h"
#include "game/component/EffectComponent.h"
#include "game/component/TransformComponent.h"
#include "core/interface/ILogger.h"

namespace game::system
{
	EffectSystem::EffectSystem(core::ecs::ComponentManager& componentManager,
		core::base::EventBus& eventBus,
		core::iface::IEffectFactory& effectFactory)
		: m_componentManager{ componentManager }
		, m_eventBus{ eventBus }
		, m_effectFactory{ effectFactory }
	{
		// AttackHitEventを購読する
		m_eventBus.subscribe<game::event::AttackHitEvent>(
			[this](const game::event::AttackHitEvent& e) {onAttackHit(e); }
		);
	}


	void EffectSystem::update(float deltaTime)
	{
		// エフェクトの自動終了チェックの処理を委譲する
		m_effectFactory.update();


		auto entities{ m_componentManager.getAllEntities<component::EffectComponent>() };
		for (auto entityId : entities)
		{
			auto& effect{ m_componentManager.get<component::EffectComponent>(entityId) };

			std::erase_if(effect.m_slots, [](const component::EffectComponent::Slot& slot)
				{
					return !slot.m_isActive;
				});
		}
	}

	void EffectSystem::onAttackHit(const game::event::AttackHitEvent& event)
	{
		// ターゲットがTransformComponentを持っていなければそもそも再生ができない
		if (!m_componentManager.has<component::TransformComponent>(event.m_targetId)) return;

		const auto& transform{ m_componentManager.get<component::TransformComponent>(event.m_targetId) };

		// エフェクトを再生してハンドルを取得する
		int handle{ m_effectFactory.play(event.m_effectType, transform.m_position) };
		if (handle == -1) return;

		if (!m_componentManager.has<component::EffectComponent>(event.m_targetId))
		{
			LOG("ターゲットにエフェクトのコンポーネントをつけてください");
			return;
		}

		// スロットに記録する
		auto& effect{ m_componentManager.get<component::EffectComponent>(event.m_targetId) };
		component::EffectComponent::Slot slot{};
		slot.m_type     = event.m_effectType;
		slot.m_handle   = handle;
		slot.m_isActive = true;
		effect.m_slots.push_back(slot);
	}
}