#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/ecs/ISystem.h"

namespace game::system
{
    /**
     * @brief ダメージヒット時の点滅演出を管理するシステム
     */
    class HitEffectSystem : public core::ecs::ISystem
    {
    public:
        /**
         * @brief コンストラクタ
         * @param componentManager コンポーネントマネージャー
         */
        explicit HitEffectSystem(core::ecs::ComponentManager& componentManager);

        /**
         * @brief システムの更新処理
         * @param deltaTime フレーム間の時間差
         */
        void update(float deltaTime) override;

    private:
        core::ecs::ComponentManager& m_componentManager;
    };
}