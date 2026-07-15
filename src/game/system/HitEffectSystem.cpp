#include "HitEffectSystem.h"
#include "game/component/HitEffectComponent.h"
#include "game/component/RenderComponent.h"

namespace game::system
{
    HitEffectSystem::HitEffectSystem(core::ecs::ComponentManager& componentManager,
        core::base::EventBus& eventBus)
        : m_componentManager{ componentManager }
        , m_eventBus{ eventBus }
    {
        m_eventBus.subscribe<game::event::AttackHitEvent>(
            [this](const game::event::AttackHitEvent& e) { onAttackHit(e); });
    }

    void HitEffectSystem::onAttackHit(const game::event::AttackHitEvent& e)
    {
        if (!m_componentManager.has<component::HitEffectComponent>(e.m_targetId))
            return;

        auto& effect{ m_componentManager.get<component::HitEffectComponent>(e.m_targetId) };
        effect.m_isActive      = true;
        effect.m_durationTimer = effect.m_duration;
        effect.m_blinkTimer    = effect.m_blinkInterval;
    }

    void HitEffectSystem::update(float deltaTime)
    {
        auto entities{ m_componentManager.getAllEntities<component::HitEffectComponent>() };
        for (auto entityId : entities)
        {
            auto& effect{ m_componentManager.get<component::HitEffectComponent>(entityId) };

            if (!effect.m_isActive)
                continue;

            // トグルタイマーを更新
            effect.m_blinkTimer -= deltaTime;
            if (effect.m_blinkTimer <= 0.0f)
            {
                effect.m_blinkTimer = effect.m_blinkInterval;
                if (m_componentManager.has<component::RenderComponent>(entityId))
                {
                    auto& render{ m_componentManager.get<component::RenderComponent>(entityId) };
                    render.m_isVisible = !render.m_isVisible;
                }
            }

            // 残り時間を更新
            effect.m_durationTimer -= deltaTime;

            // 点滅終了
            if (effect.m_durationTimer <= 0.0f)
            {
                effect.m_isActive = false;
                if (m_componentManager.has<component::RenderComponent>(entityId))
                {
                    auto& render{ m_componentManager.get<component::RenderComponent>(entityId) };
                    render.m_isVisible = true;
                }
                continue;
            }
        }
    }
} // namespace game::system