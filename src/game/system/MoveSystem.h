#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

namespace game::system
{
    /**
     * @brief 入力を元に速度を計算するSystem
     */
    class MoveSystem : public core::ecs::ISystem
    {
    public:
        MoveSystem(core::ecs::ComponentManager& componentManager, core::ecs::EntityId playerId, float moveSpeed);
        void update(float deltaTime) override;

    private:
        core::ecs::ComponentManager& m_componentManager;
        core::ecs::EntityId m_playerId;
        float m_moveSpeed;
    };
}