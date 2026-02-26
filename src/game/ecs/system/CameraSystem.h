#pragma once
#include "game/ecs/ISystem.h"
#include "game/ecs/ComponentManager.h"
#include "game/ecs/Entity.h"

namespace game::ecs::system
{
    /**
     * @brief カメラの位置を更新するSystem
     */
    class CameraSystem : public ISystem
    {
    public:
        CameraSystem(ComponentManager& componentManager, EntityId playerId);
        void update(float deltaTime) override;

    private:
        ComponentManager& m_componentManager;
        EntityId m_playerId;
    };
}