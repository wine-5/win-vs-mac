#pragma once
#include "core/ecs/Entity.h"
#include "core/event/IGameEvent.h"
#include "core/constant/EffectType.h"
#include "core/constant/SeType.h"
#include "core/data/BossMetadata.h"
#include "core/utility/Vector3.h"
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
		core::constant::EffectType m_effectType{ core::constant::EffectType::Enemy_HitSword };

		/** @brief 再生するSEの種類 */
		core::constant::SeType m_seType{ core::constant::SeType::None };

		AttackHitEvent() = default;
		AttackHitEvent(core::ecs::EntityId atkId, core::ecs::EntityId tgtId, float dmg,
		    core::constant::EffectType effectType = core::constant::EffectType::Enemy_HitSword,
		    core::constant::SeType seType = core::constant::SeType::None)
		    : m_attackerId{ atkId }
		    , m_targetId{ tgtId }
		    , m_damage{ dmg }
		    , m_effectType{ effectType }
		    , m_seType{ seType }
		{
		}
	};

	/**
	 * @brief 攻撃を開始した（振り始めた）ときに発行されるイベント
	 * ヒットの有無に関わらず、攻撃モーションの再生に合わせたエフェクト（斬撃など）を出すために使用する
	 */
	struct AttackStartEvent : public core::event::IGameEvent
	{
		/** @brief 攻撃者のEntityId */
		core::ecs::EntityId m_attackerId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief 再生するエフェクトの種類 */
		core::constant::EffectType m_effectType{ core::constant::EffectType::None };

		// TODO: ここに音も追加して敵、Playerごとに異なる音を再生するようにする予定

		AttackStartEvent() = default;
		AttackStartEvent(core::ecs::EntityId attackerId, core::constant::EffectType effectType)
		    : m_attackerId{ attackerId }
		    , m_effectType{ effectType }
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

	/**
	 * @brief 敵がスポーンしたときに発行されるイベント（初期配置・ボスの召喚の両方で発行）
	 */
	struct EnemySpawnedEvent : public core::event::IGameEvent
	{
		/** @brief スポーンした敵のEntityId */
		core::ecs::EntityId m_entityId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief スポーン位置 */
		core::Vector3 m_position{};

		EnemySpawnedEvent() = default;
		EnemySpawnedEvent(core::ecs::EntityId id, core::Vector3 position)
		    : m_entityId{ id }
		    , m_position{ position }
		{
		}
	};

	/**
	 * @brief 敵がプレイヤーを発見した（未索敵→索敵に切り替わった）瞬間に発行されるイベント
	 *
	 * DetectionSystemが敵の種類に依らず一律に検知して発行する。
	 * 発見演出（通知バッジ表示など）のトリガーに使う
	 */
	struct EnemyAlertedEvent : public core::event::IGameEvent
	{
		/** @brief 発見した敵のEntityId */
		core::ecs::EntityId m_entityId{ core::ecs::INVALID_ENTITY_ID };

		EnemyAlertedEvent() = default;
		explicit EnemyAlertedEvent(core::ecs::EntityId id)
		    : m_entityId{ id }
		{
		}
	};

	/**
	 * @brief ボスがフェーズ移行（覚醒）した瞬間に発行されるイベント
	 *
	 * 覚醒演出（カメラのズーム・シェイク・赤ビネット等）のトリガーに使う。
	 */
	struct BossPhaseTransitionEvent : public core::event::IGameEvent
	{
		/** @brief 移行したボスのEntityId */
		core::ecs::EntityId m_entityId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief 移行後のフェーズ */
		core::data::BossPhase m_newPhase{ core::data::BossPhase::Awakened };

		BossPhaseTransitionEvent() = default;
		BossPhaseTransitionEvent(core::ecs::EntityId id, core::data::BossPhase newPhase)
		    : m_entityId{ id }
		    , m_newPhase{ newPhase }
		{
		}
	};
} // namespace game::event