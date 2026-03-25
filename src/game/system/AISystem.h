#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"

namespace game::system
{
    /**
     * @brief AIComponentを持つ全エンティティのAI追従行動を処理するSystem
     */
    class AISystem : public core::ecs::ISystem
    {

    public:
        /**
        * @brief AISystemのコンストラクタ
        * @param componentManager ComponentManagerの参照
        */
        AISystem(core::ecs::ComponentManager& componentManager);

        /**
         * @brief AIの追従行動を更新する
         * @param deltaTime フレーム間の時間差
         */
        void update(float deltaTime) override;

    private:
        core::ecs::ComponentManager& m_componentManager;
    };
}