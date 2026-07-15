#pragma once
#include "core/ecs/Entity.h"
#include "core/event/IGameEvent.h"
#include "core/constant/EffectType.h"
#include "core/constant/SeType.h"
#include "game/constant/AnimationState.h"

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

		/** @brief 再生するエフェクトの種類 */
		core::constant::EffectType m_effectType{ core::constant::EffectType::Hit };

		/** @brief 再生するSEの種類 */
		core::constant::SeType m_seType{ core::constant::SeType::None };

		AttackHitEvent() = default;
		AttackHitEvent(core::ecs::EntityId atkId, core::ecs::EntityId tgtId, float dmg,
			core::constant::EffectType effectType = core::constant::EffectType::Hit,
			core::constant::SeType seType = core::constant::SeType::None)
			: m_attackerId{ atkId }
			, m_targetId{ tgtId }
			, m_damage{ dmg }
			, m_effectType{effectType}
			, m_seType{seType}
		{
		}
	};

	/**
	 * @brief 非ループアニメーションが再生完了したときに発行されるイベント
	 */
	struct AnimationFinishedEvent : public core::event::IGameEvent
	{
		/** @brief 再生が完了したEntityId */
		core::ecs::EntityId m_entityId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief 完了したアニメーション状態 */
		constant::AnimationState m_state{ constant::AnimationState::Idle };

		AnimationFinishedEvent() = default;
		AnimationFinishedEvent(core::ecs::EntityId id, constant::AnimationState state)
			: m_entityId{ id }
			, m_state{ state }
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
} // namespace game::event