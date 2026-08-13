#include "AttackSystem.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/combat/HealthComponent.h"
#include "game/component/movement/InputComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/TagComponent.h"
#include "game/component/combat/ProjectileComponent.h"
#include "game/component/visual/HitEffectComponent.h"
#include "game/component/visual/AnimationComponent.h"
#include "game/constant/Tag.h"
#include "game/attack/DamageChain.h"
#include "game/attack/BaseAttackHandler.h"
#include "game/attack/DefenseHandler.h"
#include "game/attack/CriticalHandler.h"
#include "core/interface/ILogger.h"
#include "core/utility/Log.h"
#include "game/event/InGameEvents.h"

namespace game::system::combat
{
	AttackSystem::AttackSystem(core::ecs::ComponentManager& componentManager, core::base::EventBus& eventBus)
	    : m_componentManager{ componentManager }
	    , m_eventBus{ eventBus }
	{
		// 攻撃力 → 防御力の減算 → クリティカルの倍化 の順で組む。
		// クリティカルを防御より後ろに置くのは、先に倍化すると防御の高い相手ほど
		// 減算で旨味が消えてしまい「クリティカルが出た手応え」が無くなるため
		auto base{ std::make_unique<attack::BaseAttackHandler>(m_componentManager) };
		auto defense{ std::make_unique<attack::DefenseHandler>(m_componentManager) };
		auto critical{ std::make_unique<attack::CriticalHandler>(m_componentManager) };
		defense->setNext(std::move(critical));
		base->setNext(std::move(defense));
		m_damageChain = std::move(base);
	}

	void AttackSystem::update(float deltaTime)
	{
		auto attackers{ m_componentManager.getAllEntities<component::combat::AttackComponent>() };

		for (auto attackerId : attackers)
		{
			auto& attack{ m_componentManager.get<component::combat::AttackComponent>(attackerId) };

			// 「このフレームで攻撃を開始したか／当たったか」は毎フレーム作り直す
			attack.m_justFired = false;
			attack.m_justResolved = false;

			// 死亡済みのEntityは攻撃を成立させない。AIは死亡時に止めているが、
			// 倒れる直前に立った m_attackRequested が残っていると次のフレームで死体が殴ってくる
			const auto* attackerHealth{ m_componentManager.tryGet<component::combat::HealthComponent>(attackerId) };
			if (attackerHealth != nullptr && attackerHealth->m_isDead)
			{
				attack.m_attackRequested = false;
				attack.m_windupPending = false;
				continue;
			}

			// クールダウンを更新
			if (attack.m_currentCooldown > 0.0f)
				attack.m_currentCooldown -= deltaTime;

			// ワインドアップ（溜め）中：時間を消化し、振りが終わったタイミングでダメージを解決する。
			// 溜め中は新たな攻撃要求を受け付けない。
			if (attack.m_windupPending)
			{
				attack.m_windupTimer -= deltaTime;

				// エフェクトだけは着弾より先に出す。土煙のように絵が立ち上がるまで間のある演出は、
				// 音と同時に出すと叩きつけが終わってから盛り上がってしまう
				if (!attack.m_hasPlayedImpactEffect &&
				    attack.m_impactEffectType != core::constant::EffectType::None &&
				    attack.m_windupTimer <= attack.m_impactEffectLead)
				{
					attack.m_hasPlayedImpactEffect = true;
					m_eventBus.publish(event::AttackImpactEvent{ attackerId, core::constant::SeType::None,
					    attack.m_impactEffectType });
				}

				if (attack.m_windupTimer <= 0.0f)
				{
					attack.m_windupPending = false;

					// 振り終わり＝地面を叩く瞬間。当たったかどうかに関係なく出したいので、
					// ヒット判定（resolveAttack）より前に発行する。
					// エフェクトを先出し済みならここでは音だけ鳴らす
					const core::constant::EffectType impactEffect{ attack.m_hasPlayedImpactEffect
						                                               ? core::constant::EffectType::None
						                                               : attack.m_impactEffectType };

					if (attack.m_impactSeType != core::constant::SeType::None ||
					    impactEffect != core::constant::EffectType::None)
						m_eventBus.publish(event::AttackImpactEvent{ attackerId, attack.m_impactSeType,
						    impactEffect });

					// 攻撃者が倒された場合はこのフレームへ到達しない（先頭で溜めごと打ち切る）
					attack.m_justResolved = true;
					resolveAttack(attackerId, attack);
					attack.m_currentCooldown = attack.m_attackCooldown;
				}
				continue;
			}

			// クールダウン中はまだ攻撃できないためスキップ
			if (attack.m_currentCooldown > 0.0f)
				continue;

			// 攻撃範囲が0もしくは未設定の場合はスキップ
			if (attack.m_attackRange <= 0.0f)
				continue;

			// プレイヤーの攻撃入力は PlayerAttackComboSystem が段数へ振り分けたうえで
			// m_attackRequested を立てる。敵はAI Systemが立てる。
			// 本Systemは要求を受けて成立させるだけで、入力そのものは見ない
			if (!attack.m_attackRequested)
				continue;

			attack.m_attackRequested = false;
			attack.m_justFired = true;

			// 攻撃開始時の演出用エフェクト（AttackStartEvent）の発行を絞る：
			// ・Playerの近接（剣）：斬撃エフェクトを出す。Playerの弾（Window弾）はエフェクト無し
			// ・Enemyの弾：弾自身が持つ m_startEffect を出す（None なら無し）。
			//   弾ごとに発射演出を出し分けられるようにするための仕組み。
			//   地面を叩く近接（弾でない敵攻撃）は振り始めではなく当たる瞬間に出す
			//   （弾はProjectileSystemが毎フレームm_attackRequestedを立て直すため、初回1回のみに絞る）
			const auto& attackerTagForStart{ m_componentManager.get<component::TagComponent>(attackerId) };
			const bool isProjectile{ m_componentManager.has<component::combat::ProjectileComponent>(attackerId) };

			bool shouldPlayStartEffect{ false };
			core::constant::EffectType startEffect{ core::constant::EffectType::None };

			if (attackerTagForStart.m_tag == constant::Tag::Player)
			{
				// プレイヤーは近接（剣）のときだけ斬撃エフェクト。弾（遠距離）は出さない
				if (!isProjectile)
				{
					// 剣を振るアニメーションとエフェクトは段数に応じて PlayerAttackComboSystem が要求する
					startEffect = attack.m_startEffectType;
					shouldPlayStartEffect = startEffect != core::constant::EffectType::None;
				}
			}
			else if (attackerTagForStart.m_tag == constant::Tag::Enemy)
			{
				// 敵は投擲（弾）のときだけエフェクト。地面叩き等の近接はエフェクト無し。
				// 出すエフェクトの種別は弾自身が持つ（Noneならエフェクト無し）
				if (isProjectile)
				{
					auto& projectile{ m_componentManager.get<component::combat::ProjectileComponent>(attackerId) };
					startEffect = projectile.m_startEffect;
					shouldPlayStartEffect = !projectile.m_hasPlayedStartEffect && startEffect != core::constant::EffectType::None;
					projectile.m_hasPlayedStartEffect = true;
				}
			}

			// 音と絵は別々に指定できる（振り音だけ鳴らす攻撃があるため）。
			// どちらか一方でも出すものがあればイベントを発行する
			if (shouldPlayStartEffect || attack.m_startSeType != core::constant::SeType::None)
				m_eventBus.publish(event::AttackStartEvent{ attackerId,
				    shouldPlayStartEffect ? startEffect : core::constant::EffectType::None,
				    attack.m_effectRotationOffset, attack.m_effectPositionOffset,
				    attack.m_startSeType });

			// ワインドアップ有り：振りが終わる（m_windupDelay秒後）までダメージ判定を遅延させる。
			// 演出（AttackStartEvent）は今すぐ発行済みなので、アニメの振りとダメージのタイミングが揃う。
			if (attack.m_windupDelay > 0.0f)
			{
				attack.m_windupPending = true;
				attack.m_windupTimer = attack.m_windupDelay;
				attack.m_hasPlayedImpactEffect = false;
				continue;
			}

			// ワインドアップ無し（従来動作）：発動と同時が当たる瞬間になる
			if (attack.m_impactSeType != core::constant::SeType::None ||
			    attack.m_impactEffectType != core::constant::EffectType::None)
				m_eventBus.publish(event::AttackImpactEvent{ attackerId, attack.m_impactSeType,
				    attack.m_impactEffectType });

			// 即座にダメージを解決する
			attack.m_justResolved = true;
			resolveAttack(attackerId, attack);

			// クールダウンをリセット
			attack.m_currentCooldown = attack.m_attackCooldown;
		}
	}

	void AttackSystem::resolveAttack(core::ecs::EntityId attackerId, component::combat::AttackComponent& attack)
	{
		auto targets{ m_componentManager.getAllEntities<component::combat::HealthComponent>() };
		for (auto targetId : targets)
		{
			if (targetId == attackerId)
				continue;

			// 同じ陣営同士は攻撃しないように（敵が敵を殴るフレンドリーファイア防止）
			const auto& attackerTagCheck{ m_componentManager.get<component::TagComponent>(attackerId) };
			const auto& targetTagCheck{ m_componentManager.get<component::TagComponent>(targetId) };
			if (attackerTagCheck.m_tag == targetTagCheck.m_tag)
				continue;

			if (!m_componentManager.has<component::movement::TransformComponent>(targetId))
				continue;

			// 死亡済み（後始末待ち）のEntityは攻撃対象にしない
			if (m_componentManager.get<component::combat::HealthComponent>(targetId).m_isDead)
				continue;

			// 無敵中（ボス覚醒演出など）はダメージを与えない
			if (m_componentManager.get<component::combat::HealthComponent>(targetId).m_isInvincible)
				continue;

			// プレイヤーは被弾後の点滅（HitEffect）中は無敵＝連続ヒット防止。
			// 無敵時間はHitEffectComponent.m_duration（1秒）に自動で同期する
			if (targetTagCheck.m_tag == constant::Tag::Player &&
			    m_componentManager.has<component::visual::HitEffectComponent>(targetId) &&
			    m_componentManager.get<component::visual::HitEffectComponent>(targetId).m_isActive)
				continue;

			// AttackComponentを持つEntityの攻撃範囲チェック
			auto& attackerTransform{ m_componentManager.get<component::movement::TransformComponent>(attackerId) };
			auto& targetTransform{ m_componentManager.get<component::movement::TransformComponent>(targetId) };

			float dx{ attackerTransform.m_position.x - targetTransform.m_position.x };
			float dz{ attackerTransform.m_position.z - targetTransform.m_position.z };
			float distanceSq{ dx * dx + dz * dz };
			float rangeSq{ attack.m_attackRange * attack.m_attackRange };

			if (distanceSq > rangeSq) // 攻撃範囲外の場合
				continue;

			// 高さ制限のある攻撃（地面叩きつけ等）は、相手が上限より高く浮いていたら当たらない。
			// 0なら高さ無制限なのでこのチェックは行わない
			if (attack.m_attackMaxHeight > 0.0f &&
			    targetTransform.m_position.y - attackerTransform.m_position.y > attack.m_attackMaxHeight)
				continue;

			// CORチェーンでダメージ計算を行う
			attack::DamageChain chain{};
			chain.m_attackId = attackerId;
			chain.m_targetId = targetId;
			m_damageChain->handle(chain);

			// HPを減らす
			auto& health{ m_componentManager.get<component::combat::HealthComponent>(targetId) };
			health.m_currentHp -= chain.m_damage;

			// 被ダメージのログ（攻撃者・被ダメ者・ダメージ量）
			// core::log::info("ダメージ発生: 攻撃者={} 被ダメ者={} ダメージ={:.1f}",
			//     static_cast<unsigned int>(attackerId),
			//     static_cast<unsigned int>(targetId),
			//     chain.m_damage);

			if (health.m_currentHp < 0.0f)
				health.m_currentHp = 0.0f;

			// 死亡判定と死亡の場合はエンティティに応じたイベントを発行する
			if (health.m_currentHp <= 0.0f && !health.m_isDead)
			{
				health.m_isDead = true;
				auto& tag{ m_componentManager.get<component::TagComponent>(targetId) };
				if (tag.m_tag == constant::Tag::Player)
					m_eventBus.publish(event::PlayerDeadEvent{});
				else if (tag.m_tag == constant::Tag::Enemy)
					m_eventBus.publish(event::EnemyDeadEvent{ targetId });
			}

			// AttackHitイベントを発行する
			event::AttackHitEvent hitEvent{};
			hitEvent.m_attackerId = attackerId;
			hitEvent.m_targetId = targetId;
			hitEvent.m_damage = chain.m_damage;
			hitEvent.m_isCritical = chain.m_isCritical;

			// 被弾エフェクトは「誰が食らったか」で決める。
			// 敵が食らったときだけ、当たったのが弾（Enemy_HitWindow）か剣（Enemy_HitSword系）かで出し分ける。
			// 剣の場合はさらに、その一振りが近接コンボの何段目だったかで通常／強を切り替える。
			// 段数そのもの（AttackComboComponent.m_stage）は受付時間切れで振っている最中に0へ戻り得るため、
			// 一振りごとに確定して以降上書きされない m_startEffectType を段の判定に使う
			if (targetTagCheck.m_tag == constant::Tag::Player)
				hitEvent.m_effectType = core::constant::EffectType::Player_Hit;
			else if (m_componentManager.has<component::combat::ProjectileComponent>(attackerId))
				hitEvent.m_effectType = core::constant::EffectType::Enemy_HitWindow;
			else
				hitEvent.m_effectType = attack.m_startEffectType == core::constant::EffectType::Player_SlashStrong
				                            ? core::constant::EffectType::Enemy_HitSwordStrong
				                            : core::constant::EffectType::Enemy_HitSwordNormal;

			// ヒット音は「何が当たったか」で決める。弾は弾自身が持つ音（Window弾・溜め撃ちで別）、
			// プレイヤーの近接は敵が斬られた音。振り音は AttackStartEvent 側が担う
			const auto& attackerTag{ m_componentManager.get<component::TagComponent>(attackerId) };
			if (const auto* projectile{ m_componentManager.tryGet<component::combat::ProjectileComponent>(attackerId) })
				hitEvent.m_seType = projectile->m_hitSeType;
			else if (attackerTag.m_tag == constant::Tag::Player)
				hitEvent.m_seType = core::constant::SeType::HitEnemy;

			m_eventBus.publish(hitEvent);
		}
	}
} // namespace game::system::combat