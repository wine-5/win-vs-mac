#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/ecs/ISystem.h"
#include "core/base/EventBus.h"
#include "core/interface/IEffectFactory.h"
#include "game/event/InGameEvents.h"
#include <vector>

namespace game::system::visual
{
    /**
     * @brief AttackHitEvent を受けてエフェクトを再生するシステム
     */
    class EffectSystem : public core::ecs::ISystem
    {
    public:
        EffectSystem(core::ecs::ComponentManager& componentManager,
            core::base::EventBus& eventBus,
            core::iface::IEffectFactory& effectFactory);

        /**
         * @brief システムの更新処理（エフェクト終了スロットの回収）
         * @param deltaTime フレーム間の時間差
         */
        void update(float deltaTime) override;

    private:
        /**
         * @brief コンストラクタから呼び出す、各イベントの購読設定
         */
	  void setupEventSubscriptions();

	  void onAttackHit(const game::event::AttackHitEvent& event);
	  void onAttackStart(const game::event::AttackStartEvent& event);
	  void onEnemyDead(const game::event::EnemyDeadEvent& event);
	  void onEnemySpawned(const game::event::EnemySpawnedEvent& event);

	  core::ecs::ComponentManager& m_componentManager;
	  core::base::EventBus& m_eventBus;
	  core::iface::IEffectFactory& m_effectFactory;

	  // EventBusの購読ハンドル。このクラスが破棄されると自動で解除される
	  std::vector<core::base::EventBus::Subscription> m_subscriptions{};
	};
} // namespace game::system::visual