#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/EntityManager.h"
#include "core/ecs/Entity.h"
#include "core/base/EventBus.h"

namespace game::system::stage
{
	/**
	 * @brief 落ちている拡張子の欠片を動かし、拾わせるSystem
	 *
	 * 欠片はブロックから弾け出て落下し、着地したらその場で浮遊する。
	 * プレイヤーが一定距離まで近づくと吸い寄せられ、触れると取得する。
	 *
	 * 吸い寄せるのは、拾うために正確な位置合わせを強いないため。
	 * 戦闘中に足元を見ながら歩く操作は面倒なだけで、上手さの表現にならない。
	 */
	class ExtensionPickupSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief ExtensionPickupSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param entityManager 拾った欠片を破棄するためのEntityManager
		 * @param eventBus 取得を知らせるためのEventBus
		 * @param playerId プレイヤーのEntityID
		 */
		ExtensionPickupSystem(core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityManager& entityManager,
		    core::base::EventBus& eventBus,
		    core::ecs::EntityId playerId);

		/**
		 * @brief 欠片を落下・浮遊させ、近づいたら取得する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityManager& m_entityManager;
		core::base::EventBus& m_eventBus;
		core::ecs::EntityId m_playerId;
	};
} // namespace game::system::stage
