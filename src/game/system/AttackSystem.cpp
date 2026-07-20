#include "AttackSystem.h"
#include "game/component/AttackComponent.h"
#include "game/component/HealthComponent.h"
#include "game/component/InputComponent.h"
#include "game/component/TransformComponent.h"
#include "game/component/TagComponent.h"
#include "game/component/ProjectileComponent.h"
#include "game/component/HitEffectComponent.h"
#include "game/constant/Tag.h"
#include "game/attack/DamageChain.h"
#include "game/attack/BaseAttackHandler.h"
#include "game/attack/DefenseHandler.h"
#include "core/interface/ILogger.h"
#include "game/event/InGameEvents.h"

namespace game::system
{
	AttackSystem::AttackSystem(core::ecs::ComponentManager &componentManager, core::base::EventBus &eventBus,
		core::constant::SeType playerAttackSeType)
		: m_componentManager{componentManager}, m_eventBus{eventBus}, m_playerAttackSeType{playerAttackSeType}
	{
		auto base{std::make_unique<attack::BaseAttackHandler>(m_componentManager)};
		auto defense{std::make_unique<attack::DefenseHandler>(m_componentManager)};
		base->setNext(std::move(defense));
		m_damageChain = std::move(base);
	}

	void AttackSystem::update(float deltaTime)
	{
		auto attackers{m_componentManager.getAllEntities<component::AttackComponent>()};

		for (auto attackerId : attackers)
		{
			auto &attack{m_componentManager.get<component::AttackComponent>(attackerId)};

			// クールダウンを更新
			if (attack.m_currentCooldown > 0.0f)
			{
				attack.m_currentCooldown -= deltaTime;
				continue; // まだ攻撃できないためスキップ
			}

			// 攻撃範囲が0もしくは未設定の場合はスキップ
			if (attack.m_attackRange <= 0.0f)
				continue;

			if (m_componentManager.has<component::InputComponent>(attackerId))
			{

				auto &input{m_componentManager.get<component::InputComponent>(attackerId)};
				if (input.m_attackPressed)
					attack.m_attackRequested = true;
			}

			if (!attack.m_attackRequested)
				continue;

			attack.m_attackRequested = false;

			// 攻撃を開始した瞬間に再生する演出用エフェクト（ヒットの有無に関わらず発行）
			// Playerの剣攻撃（左クリック）はPlayer_Slash、Enemyの攻撃（近接/遠距離問わず）はEnemy_Slashを再生する
			const auto& attackerTagForStart{ m_componentManager.get<component::TagComponent>(attackerId) };
			if (attackerTagForStart.m_tag == constant::Tag::Player)
				m_eventBus.publish(event::AttackStartEvent{ attackerId, core::constant::EffectType::Player_Slash });
			else if (attackerTagForStart.m_tag == constant::Tag::Enemy)
				m_eventBus.publish(event::AttackStartEvent{ attackerId, core::constant::EffectType::Enemy_Slash });

			auto targets{m_componentManager.getAllEntities<component::HealthComponent>()};
			for (auto targetId : targets)
			{
				if (targetId == attackerId)
					continue;

				// 同じ陣営同士は攻撃しないように（敵が敵を殴るフレンドリーファイア防止）
				const auto& attackerTagCheck{ m_componentManager.get<component::TagComponent>(attackerId) };
				const auto& targetTagCheck{ m_componentManager.get<component::TagComponent>(targetId) };
				if (attackerTagCheck.m_tag == targetTagCheck.m_tag)
					continue;

				if (!m_componentManager.has<component::TransformComponent>(targetId))
					continue;

				// 無敵中（ボス覚醒演出など）はダメージを与えない
				if (m_componentManager.get<component::HealthComponent>(targetId).m_isInvincible)
					continue;

				// プレイヤーは被弾後の点滅（HitEffect）中は無敵＝連続ヒット防止。
				// 無敵時間はHitEffectComponent.m_duration（1秒）に自動で同期する
				if (targetTagCheck.m_tag == constant::Tag::Player &&
				    m_componentManager.has<component::HitEffectComponent>(targetId) &&
				    m_componentManager.get<component::HitEffectComponent>(targetId).m_isActive)
					continue;

				// AttackComponentを持つEntityの攻撃範囲チェック
				auto &attackerTransform{m_componentManager.get<component::TransformComponent>(attackerId)};
				auto &targetTransform{m_componentManager.get<component::TransformComponent>(targetId)};

				float dx{attackerTransform.m_position.x - targetTransform.m_position.x};
				float dz{attackerTransform.m_position.z - targetTransform.m_position.z};
				float distanceSq{dx * dx + dz * dz};
				float rangeSq{attack.m_attackRange * attack.m_attackRange};

				if (distanceSq > rangeSq) // 攻撃範囲外の場合
					continue;

				// CORチェーンでダメージ計算を行う
				attack::DamageChain chain{};
				chain.m_attackId = attackerId;
				chain.m_targetId = targetId;
				m_damageChain->handle(chain);

				// HPを減らす
				auto &health{m_componentManager.get<component::HealthComponent>(targetId)};
				health.m_currentHp -= chain.m_damage;

				// 被ダメージのログ（攻撃者・被ダメ者・ダメージ量）
				// LOG("ダメージ発生: 攻撃者={} 被ダメ者={} ダメージ={:.1f}",
				//     static_cast<unsigned int>(attackerId),
				//     static_cast<unsigned int>(targetId),
				//     chain.m_damage);

				if (health.m_currentHp < 0.0f)
					health.m_currentHp = 0.0f;

				// 死亡判定と死亡の場合はエンティティに応じたイベントを発行する
				if (health.m_currentHp <= 0.0f && !health.m_isDead)
				{
					health.m_isDead = true;
					auto &tag{m_componentManager.get<component::TagComponent>(targetId)};
					if (tag.m_tag == constant::Tag::Player)
						m_eventBus.publish(event::PlayerDeadEvent{});
					else if (tag.m_tag == constant::Tag::Enemy)
						m_eventBus.publish(event::EnemyDeadEvent{targetId});
				}

				// AttackHitイベントを発行する
				event::AttackHitEvent hitEvent{};
				hitEvent.m_attackerId = attackerId;
				hitEvent.m_targetId = targetId;
				hitEvent.m_damage = chain.m_damage;

				// 攻撃者がProjectileComponentを持つ（=弾＝Window投撃などの遠距離攻撃）ならEnemy_HitWindow、
				// そうでなければ（=本体による近接攻撃）Enemy_HitSwordを再生する
				hitEvent.m_effectType = m_componentManager.has<component::ProjectileComponent>(attackerId)
				                            ? core::constant::EffectType::Enemy_HitWindow
				                            : core::constant::EffectType::Enemy_HitSword;

				// 攻撃者がプレイヤーの場合、ジョブに応じた攻撃SEをセット
				const auto& attackerTag{ m_componentManager.get<component::TagComponent>(attackerId) };
				if (attackerTag.m_tag == constant::Tag::Player)
					hitEvent.m_seType = m_playerAttackSeType;

				m_eventBus.publish(hitEvent);
			}

			// クールダウンをリセット
			attack.m_currentCooldown = attack.m_attackCooldown;
		}
	}
} // namespace game::system