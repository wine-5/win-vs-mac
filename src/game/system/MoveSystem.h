#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

namespace game::ecs::system
{
    /**
     * @brief 入力を元に速度を計算するSystem
     */
    class MoveSystem : public ISystem
    {
    public:
        MoveSystem(ComponentManager& componentManager, EntityId playerId, float moveSpeed);
        void update(float deltaTime) override;

    private:
        ComponentManager& m_componentManager;
        EntityId m_playerId;
        float m_moveSpeed;
    };
}