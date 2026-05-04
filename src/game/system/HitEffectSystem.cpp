#include "HitEffectSystem.h"
#include "game/component/HitEffectComponent.h"
#include "game/component/RenderComponent.h"

namespace game::system
{
    HitEffectSystem::HitEffectSystem(core::ecs::ComponentManager& componentManager)
        : m_componentManager{ componentManager }
    {
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
}