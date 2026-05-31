#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/ecs/ISystem.h"
#include "core/base/EventBus.h"
#include "game/event/InGameEvents.h"

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
         * @param eventBus EventBusの参照
         */
        HitEffectSystem(core::ecs::ComponentManager& componentManager,
            core::base::EventBus& eventBus);

        /**
         * @brief システムの更新処理
         * @param deltaTime フレーム間の時間差
         */
        void update(float deltaTime) override;

    private:
        void onAttackHit(const game::event::AttackHitEvent& e);

        core::ecs::ComponentManager& m_componentManager;
        core::base::EventBus&        m_eventBus;
    };
}