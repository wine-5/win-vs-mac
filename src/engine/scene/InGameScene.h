#pragma once
#include "IScene.h"
#include "game/ecs/EntityManager.h"
#include "game/ecs/ComponentManager.h"
#include "game/ecs/SystemManager.h"
#include "game/ObjectFactory.h"
#include "game/ecs/component/RenderComponent.h"

namespace engine::scene
{
    /**
     * @brief インゲームのシーンクラス
     */
    class InGameScene : public IScene
    {
    public:
        static constexpr float PLAYER_MOVE_SPEED = 5.0f;
        InGameScene();
        void update(float deltaTime) override;

    private:
        game::ecs::EntityManager    m_entityManager;
        game::ecs::ComponentManager m_componentManager;
        game::ecs::SystemManager    m_systemManager;
        game::ObjectFactory         m_objectFactory;
    };
}