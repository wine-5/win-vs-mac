#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "game/actor/Player.h"
#include <memory>

namespace game
{
    /**
     * @brief ゲームオブジェクトの生成・破棄を担当するクラス
     */
    class ObjectFactory
    {
    public:
        ObjectFactory(ecs::EntityManager& entityManager, ecs::ComponentManager& componentManager);
        void init();

        actor::Player& getPlayer() const;

    private:
        ecs::EntityManager& m_entityManager;
        ecs::ComponentManager& m_componentManager;
        std::unique_ptr<actor::Player> m_player; // init()で生成タイミングを遅らせるためunique_ptrで保持
    };
}