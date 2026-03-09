#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include <vector>

namespace game::system
{
    /**
     * @brief ColliderComponentを持つ全エンティティ間の衝突検出と押し返しを行うSystem
     */
    class CollisionSystem : public core::ecs::ISystem
    {
    public:
        CollisionSystem(core::ecs::ComponentManager& componentManager);

        void addEntity(core::ecs::EntityId id);
        void update(float deltaTime) override;

    private:
        // AABBの衝突判定
        bool isColliding(core::ecs::EntityId a, core::ecs::EntityId b)const;

        // 押し返し処理
        void resolveCollision(core::ecs::EntityId player, core::ecs::EntityId ground);

        core::ecs::ComponentManager& m_componentManager;
        std::vector<core::ecs::EntityId> m_entities;
    };
}