#pragma once
#include "core/ecs/Entity.h"
#include "core/interface/IGameEvent.h"
#include "core/constant/EffectType.h"
#include "core/constant/SeType.h"
#include "core/data/MacMetadata.h"
#include "core/utility/Vector3.h"
#include "game/constant/AnimationState.h"
#include "game/constant/EnemyType.h"

namespace game::event
{
	/**
	 * @brief 攻撃がヒットしたときに発行されるイベント
	 */
	struct AttackHitEvent : public core::iface::IGameEvent
	{
		/** @brief 攻撃者のEntityId */
		core::ecs::EntityId m_attackerId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief 被攻撃者のEntityId */
		core::ecs::EntityId m_targetId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief 最終的に与えたダメージ値 */
		float m_damage{ 0.0f };

		/** @brief 再生するエフェクトの種類 */
		core::constant::EffectType m_effectType{ core::constant::EffectType::Enemy_HitSwordNormal };

		/** @brief 再生するSEの種類 */
		core::constant::SeType m_seType{ core::constant::SeType::None };

		/** @brief クリティカルだったか（ダメージ数値の見せ方を変えるのに使う） */
		bool m_isCritical{ false };

		AttackHitEvent() = default;
		AttackHitEvent(core::ecs::EntityId atkId, core::ecs::EntityId tgtId, float dmg,
		    core::constant::EffectType effectType = core::constant::EffectType::Enemy_HitSwordNormal,
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
	struct AttackStartEvent : public core::iface::IGameEvent
	{
		/** @brief 攻撃者のEntityId */
		core::ecs::EntityId m_attackerId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief 再生するエフェクトの種類 */
		core::constant::EffectType m_effectType{ core::constant::EffectType::None };

		/**
		 * @brief エフェクトの向きの補正（ラジアン）
		 *
		 * 購読側は「攻撃者の向き＋この補正」でエフェクトを出す。
		 * 同じ斬撃エフェクトを縦振りと水平回転で使い分けるために持つ
		 */
		core::Vector3 m_effectRotationOffset{};

		/**
		 * @brief エフェクトの位置の補正（ワールド単位）
		 *
		 * 購読側は「基準位置＋この補正」でエフェクトを出す。
		 * エフェクトの絵柄が原点からどう伸びるかに合わせて高さを詰めるために持つ
		 */
		core::Vector3 m_effectPositionOffset{};

		/**
		 * @brief 振り始めに鳴らすSEの種類
		 *
		 * エフェクトとは独立に指定できる（音だけ・絵だけの攻撃があるため）。
		 * Noneなら無音。AudioEventListenerが購読して鳴らす
		 */
		core::constant::SeType m_seType{ core::constant::SeType::None };

		AttackStartEvent() = default;
		AttackStartEvent(core::ecs::EntityId attackerId, core::constant::EffectType effectType,
		    core::Vector3 effectRotationOffset = {}, core::Vector3 effectPositionOffset = {},
		    core::constant::SeType seType = core::constant::SeType::None)
		    : m_attackerId{ attackerId }
		    , m_effectType{ effectType }
		    , m_effectRotationOffset{ effectRotationOffset }
		    , m_effectPositionOffset{ effectPositionOffset }
		    , m_seType{ seType }
		{
		}
	};

	/**
	 * @brief 攻撃のダメージ判定が成立する瞬間に発行されるイベント
	 *
	 * 振り始め（AttackStartEvent）とは別に、「当たる瞬間」に合わせたい演出のために持つ。
	 * ワインドアップ有りの攻撃では振り終わり、無しの攻撃では発動と同時に発行される。
	 * 相手に当たったかどうかは問わない（空振りでも地面を叩く音は鳴ってほしいため）
	 */
	struct AttackImpactEvent : public core::iface::IGameEvent
	{
		/** @brief 攻撃者のEntityId */
		core::ecs::EntityId m_attackerId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief 再生するSEの種類（Noneなら無音） */
		core::constant::SeType m_seType{ core::constant::SeType::None };

		/** @brief 再生するエフェクトの種類（Noneなら演出無し） */
		core::constant::EffectType m_effectType{ core::constant::EffectType::None };

		AttackImpactEvent() = default;
		AttackImpactEvent(core::ecs::EntityId attackerId, core::constant::SeType seType,
		    core::constant::EffectType effectType = core::constant::EffectType::None)
		    : m_attackerId{ attackerId }
		    , m_seType{ seType }
		    , m_effectType{ effectType }
		{
		}
	};

	/**
	 * @brief 非ループアニメーションが再生完了したときに発行されるイベント
	 */
	struct AnimationFinishedEvent : public core::iface::IGameEvent
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
	 *
	 * これは「HPが尽きた瞬間」であって、シーンを切り替えてよい合図ではない。
	 * 死亡アニメと暗転演出を挟むため、遷移は PlayerDeathSequenceFinishedEvent を待つ
	 */
	struct PlayerDeadEvent : public core::iface::IGameEvent
	{
		PlayerDeadEvent() = default;
	};

	/**
	 * @brief プレイヤーの死亡演出（死亡アニメ→暗転）が完了したときに発行されるイベント
	 *
	 * PlayerDeathSystemが発行する。リザルトへのシーン遷移はこれを合図に行う
	 */
	struct PlayerDeathSequenceFinishedEvent : public core::iface::IGameEvent
	{
		PlayerDeathSequenceFinishedEvent() = default;
	};

	/**
	 * @brief 敵が死亡したときに発行されるイベント
	 */
	struct EnemyDeadEvent : public core::iface::IGameEvent
	{
		/** @brief 死亡した敵のEntityId */
		core::ecs::EntityId m_entityId{ core::ecs::INVALID_ENTITY_ID };

		EnemyDeadEvent() = default;
		EnemyDeadEvent(core::ecs::EntityId id) : m_entityId(id) {}
	};

	/**
	 * @brief 敵が死亡演出（死亡アニメ＋消失フェード）を終えて完全に消滅した瞬間に発行されるイベント
	 *
	 * HPが尽きた瞬間（EnemyDeadEvent）ではなく、EnemyDeathSystemがEntityを破棄する直前に発行する。
	 * ボス撃破の勝利遷移など「敵が見た目上も消えてから」進めたい処理のトリガーに使う。
	 */
	struct EnemyVanishedEvent : public core::iface::IGameEvent
	{
		/** @brief 消滅した敵のEntityId */
		core::ecs::EntityId m_entityId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief 消滅した敵の種類（Mac＝ボスかどうかを購読側が明示的に判定できるように持たせる） */
		constant::EnemyType m_type{ constant::EnemyType::Xcode };

		EnemyVanishedEvent() = default;
		EnemyVanishedEvent(core::ecs::EntityId id, constant::EnemyType type)
		    : m_entityId{ id }
		    , m_type{ type }
		{
		}
	};

	/**
	 * @brief 敵がスポーンしたときに発行されるイベント（初期配置・ボスの召喚の両方で発行）
	 */
	struct EnemySpawnedEvent : public core::iface::IGameEvent
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
	struct EnemyAlertedEvent : public core::iface::IGameEvent
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
	struct MacPhaseTransitionEvent : public core::iface::IGameEvent
	{
		/** @brief 移行したボスのEntityId */
		core::ecs::EntityId m_entityId{ core::ecs::INVALID_ENTITY_ID };

		/** @brief 移行後のフェーズ */
		core::data::MacPhase m_newPhase{ core::data::MacPhase::Awakened };

		MacPhaseTransitionEvent() = default;
		MacPhaseTransitionEvent(core::ecs::EntityId id, core::data::MacPhase newPhase)
		    : m_entityId{ id }
		    , m_newPhase{ newPhase }
		{
		}
	};

	/**
	 * @brief 雑魚を全滅させてボスが出現した瞬間に発行されるイベント
	 *
	 * 出現シネマ（カメラをボスへ寄せてシェイク→プレイヤーへ戻す）のトリガーに使う。
	 * フェーズ移行（MacPhaseTransitionEvent）とは意味が異なるため別イベントにしている
	 * ＝出現と覚醒で演出の強さ・タイミングを個別に調整できる。
	 */
	struct BossAppearedEvent : public core::iface::IGameEvent
	{
		/** @brief 出現したボスのEntityId */
		core::ecs::EntityId m_entityId{ core::ecs::INVALID_ENTITY_ID };

		BossAppearedEvent() = default;
		explicit BossAppearedEvent(core::ecs::EntityId id)
		    : m_entityId{ id }
		{
		}
	};
} // namespace game::event