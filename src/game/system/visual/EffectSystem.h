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

	  /**
	   * @brief エフェクトを再生し、寿命追跡用のスロットへ記録する
	   *
	   * 4つのイベントハンドラで共通する「再生 → 失敗なら中断 → スロットへ記録」をまとめたもの。
	   * ハンドラ側は再生位置の求め方だけを持つ。
	   * @param entityId スロットを持つEntityのID
	   * @param type 再生するエフェクトの種類
	   * @param position 再生位置（ワールド座標）
	   */
	  void playAndTrack(core::ecs::EntityId entityId,
		  core::constant::EffectType type, const core::Vector3& position);

	  core::ecs::ComponentManager& m_componentManager;
	  core::base::EventBus& m_eventBus;
	  core::iface::IEffectFactory& m_effectFactory;

	  // EventBusの購読ハンドル。このクラスが破棄されると自動で解除される
	  std::vector<core::base::EventBus::Subscription> m_subscriptions{};
	};
} // namespace game::system::visual