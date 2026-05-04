#pragma once
#include "core/ecs/Entity.h"

namespace game::event
{
	/**
	 * @brief 攻撃がヒットしたときに発行されるイベント
	 */
	struct AttackHitEvent
	{
		/** @brief 攻撃者のEntityId */
		core::ecs::EntityId m_attackerId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief 被攻撃者のEntityId */
		core::ecs::EntityId m_targetId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief 最終的に与えたダメージ値 */
		float m_damage{ 0.0f };
	};

	/**
	 * @brief プレイヤーが死亡したときに発行されるイベント
	 */
	struct PlayerDeadEvent
	{
	};

	/**
	 * @brief 敵が死亡したときに発行されるイベント
	 */
	struct EnemyDeadEvent
	{
		/** @brief 死亡した敵のEntityId */
		core::ecs::EntityId m_entityId{ core::ecs::INVALID_ENTITY_ID };
	};
}