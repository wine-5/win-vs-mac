#pragma once
#include "game/ecs/EntityManager.h"
#include "game/ecs/ComponentManager.h"
#include "game/ecs/Entity.h"

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