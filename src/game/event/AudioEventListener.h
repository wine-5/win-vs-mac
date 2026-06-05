#pragma once
#include "core/base/EventBus.h"
#include "core/ecs/Entity.h"
#include "game/event/InGameEvents.h"

namespace game::event
{
	/**
	 * @brief AttackHitEvent / EnemyDeadEvent を購読してSEを再生するリスナー
     * AudioManagerでイベントの購読を検討したがアーキテクチャ違反のためListenerクラス作成した
	 */
	class AudioEventListener
	{
	public:
		AudioEventListener() = default;

		/**
		 * @brief コンストラクタ。EventBus に購読を登録する
		 * @param eventBus EventBusの参照
		 * @param playerId プレイヤーのEntityId（被弾SE判定に使用）
		 */
		AudioEventListener(core::base::EventBus& eventBus, core::ecs::EntityId playerId);

	private:
		void onAttackHit(const AttackHitEvent& e);
		void onEnemyDead(const EnemyDeadEvent& e);

		core::base::EventBus&  m_eventBus;
		core::ecs::EntityId    m_playerId{};
	};
}
