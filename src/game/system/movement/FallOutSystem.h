#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/base/EventBus.h"

namespace game::system::movement
{
	/**
	 * @brief 奈落へ落ちた者を始末するSystem
	 *
	 * 床だけが虚無に浮かぶ構成のため、縁から落ちるとどこまでも落下する。
	 * 全周を壁で囲むと世界観が壊れるので、落下を検知して決着をつける。
	 *
	 * 落下量は接地のたびに更新される FallRecoveryComponent の位置を基準に見るので、
	 * 位置が確定したあと（GroundingSystemより後）に更新すること。
	 */
	class FallOutSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief コンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param eventBus 死亡イベント発行用のEventBusの参照
		 */
		FallOutSystem(core::ecs::ComponentManager& componentManager, core::base::EventBus& eventBus);

		/**
		 * @brief 奈落へ落ちたEntityを検知して始末する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		/**
		 * @brief 最後の足場から奈落と言える高さまで落ちたか
		 * @param entityId 判定対象のEntityID
		 * @return 奈落へ落ちたとみなせる場合true
		 */
		bool hasFallenOut(core::ecs::EntityId entityId) const;

		/**
		 * @brief 落下したプレイヤーにダメージを与え、直前の足場へ戻す
		 * @param entityId プレイヤーのEntityID
		 */
		void punishPlayer(core::ecs::EntityId entityId);

		/**
		 * @brief 落下した敵を撃破扱いにする
		 * @param entityId 敵のEntityID
		 */
		void killEnemy(core::ecs::EntityId entityId);

		core::ecs::ComponentManager& m_componentManager;
		core::base::EventBus& m_eventBus;
	};
} // namespace game::system::movement
