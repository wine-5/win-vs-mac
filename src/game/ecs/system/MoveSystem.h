#pragma once
#include "game/ecs/ISystem.h"
#include "game/ecs/ComponentManager.h"
#include "game/ecs/Entity.h"

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