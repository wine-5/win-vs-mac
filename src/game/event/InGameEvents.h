#pragma once
#include "core/ecs/Entity.h"

namespace game::event
{
    /**
     * @brief 攻撃がヒットしたときに発行されるイベント
     */
    struct AttackHitEvent
    {
        /** @brief 攻撃者のEntityId */
        core::ecs::EntityId m_attackerId{ core::ecs::INVALID_ENTITY_ID };

        /** @brief 被攻撃者のEntityId */
        core::ecs::EntityId m_targetId{ core::ecs::INVALID_ENTITY_ID };

        /** @brief 最終的に与えたダメージ値 */
        float m_damage{ 0.0f };
    };
}