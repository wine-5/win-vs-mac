#pragma once
#include <vector>
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/EntityManager.h"
#include "core/ecs/Entity.h"
#include "core/base/EventBus.h"

namespace game::system::combat
{
	/**
	 * @brief 弾の寿命管理・ヒット時の破棄を担うSystem
	 *
	 * 当たり判定とダメージは AttackSystem に任せ、本Systemは弾に固有の処理だけを行う：
	 * ・寿命を減算し、尽きたら破棄する
	 * ・AttackSystemに拾わせ続けるため毎フレーム m_attackRequested を立て直す
	 * ・AttackHitEventを購読し、攻撃者が弾ならヒットとして破棄する
	 */
	class ProjectileSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief ProjectileSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param entityManager EntityManagerの参照（破棄に使用）
		 * @param eventBus AttackHitEvent購読用のEventBus
		 */
		ProjectileSystem(core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityManager& entityManager,
		    core::base::EventBus& eventBus);

		/**
		 * @brief 寿命更新・再アーム・破棄を行う
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityManager& m_entityManager;
		core::base::EventBus& m_eventBus;

		// 破棄予約（ヒットまたは寿命切れ）。イテレーション中の即時破棄を避けるため一旦貯める
		std::vector<core::ecs::EntityId> m_pendingDestroy{};

		// EventBusの購読ハンドル。このクラスが破棄されると自動で解除される
		std::vector<core::base::EventBus::Subscription> m_subscriptions{};
	};
} // namespace game::system::combat
