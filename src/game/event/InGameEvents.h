#pragma once
#include "core/ecs/Entity.h"
#include "core/event/IGameEvent.h"

namespace game::event
{
	/**
	 * @brief 攻撃がヒットしたときに発行されるイベント
	 */
	struct AttackHitEvent : public core::event::IGameEvent
	{
		/** @brief 攻撃者のEntityId */
		core::ecs::EntityId m_attackerId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief 被攻撃者のEntityId */
		core::ecs::EntityId m_targetId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief 最終的に与えたダメージ値 */
		float m_damage{ 0.0f };

		AttackHitEvent() = default;
		AttackHitEvent(core::ecs::EntityId atkId, core::ecs::EntityId tgtId, float dmg)
			: m_attackerId{atkId}
			, m_targetId{tgtId}
			, m_damage{dmg} 
		{
		}
	};

	/**
	 * @brief プレイヤーが死亡したときに発行されるイベント
	 */
	struct PlayerDeadEvent : public core::event::IGameEvent
	{
		PlayerDeadEvent() = default;
	};

	/**
	 * @brief 敵が死亡したときに発行されるイベント
	 */
	struct EnemyDeadEvent : public core::event::IGameEvent
	{
		/** @brief 死亡した敵のEntityId */
		core::ecs::EntityId m_entityId{ core::ecs::INVALID_ENTITY_ID };

		EnemyDeadEvent() = default;
		EnemyDeadEvent(core::ecs::EntityId id) : m_entityId(id) {}
	};
}