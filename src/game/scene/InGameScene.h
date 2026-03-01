#pragma once
#include "IScene.h"
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/SystemManager.h"
#include "game/ObjectFactory.h"
#include "game/component/RenderComponent.h"

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