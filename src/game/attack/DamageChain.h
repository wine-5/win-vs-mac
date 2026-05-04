#pragma once
#include "core/ecs/Entity.h"

namespace game::attack
{
	/**
	 * @brief 攻撃計算に必要なコンテキストデータを保持する構造体
	 */
	struct DamageChain
	{
		/** @brief 攻撃者のEntityId */
		core::ecs::EntityId m_attackId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief 攻撃対象のEntityId */
		core::ecs::EntityId m_targetId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief 計算中のダメージ値(各ハンドラが段階的に加工する) */
		float m_damage{};
	};
}