#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

namespace game::actor
{
    /**
     * @brief Playerのセットアップを担当するクラス
     */
    class Player
    {
    public:
        Player(ecs::EntityManager& entityManager, ecs::ComponentManager& componentManager);
        ecs::EntityId getId() const;

    private:
        ecs::Entity m_entity;
    };
}