#include "BossAISystem.h"
#include "game/component/AIComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/AttackComponent.h"
#include "game/component/HealthComponent.h"
#include "game/component/AnimationComponent.h"
#include "game/constant/AnimationState.h"
#include "game/constant/Tag.h"
#include "game/constant/EnemyType.h"
#include "game/event/InGameEvents.h"
#include <cmath>
#include <utility>
#include <algorithm>
#include <numbers>

namespace
{
	constexpr float PI{ std::numbers::pi_v<float> };
	constexpr float DEG_TO_RAD{ PI / 180.0f };

	// 各アクションの再生ロック時間（秒）。この間は次の行動を抽選しない（アニメ・演出の尺）
	constexpr float MELEE_LOCK{ 1.0f };
	constexpr float RANGED_LOCK{ 1.2f };
	constexpr float SUMMON_LOCK{ 1.2f };
	constexpr float PHASE_TRANSITION_LOCK{ 2.0f }; // 覚醒演出の停止時間
} // namespace

namespace game::system::ai
{
	BossAISystem::BossAISystem(core::ecs::ComponentManager& componentManager,
	    core::base::EventBus& eventBus,
	    factory::ProjectileFactory& projectileFactory,
	    factory::EnemySpawner& enemySpawner,
	    core::data::ProjectileMetadata rainbowMeta,
	    int rainbowModelHandle,
	    float rainbowRadius)
	    : m_componentManager{ componentManager }
	    , m_eventBus{ eventBus }
	    , m_projectileFactory{ projectileFactory }
	    , m_enemySpawner{ enemySpawner }
	    , m_rainbowMeta{ std::move(rainbowMeta) }
	    , m_rainbowModelHandle{ rainbowModelHandle }
	    , m_rainbowRadius{ rainbowRadius }
	{
	}

	void BossAISystem::update(float deltaTime)
	{
		using component::ai::BossState;

		auto entities{ m_componentManager.getAllEntities<component::ai::BossAIComponent>() };
		for (auto entityId : entities)
		{
			auto& boss{ m_componentManager.get<component::ai::BossAIComponent>(entityId) };

			// フェーズ定義が無い（macData.jsonのboss未設定）なら何もしない
			if (boss.m_config.m_phase1.m_actionInterval <= 0.0f)
				continue;

			auto& health{ m_componentManager.get<component::HealthComponent>(entityId) };
			auto& transform{ m_componentManager.get<component::TransformComponent>(entityId) };
			auto& ai{ m_componentManager.get<component::AIComponent>(entityId) };

			const bool hasVelocity{ m_componentManager.has<component::VelocityComponent>(entityId) };

			// --- 死亡 ---
			if (health.m_isDead || boss.m_state == BossState::Dead)
			{
				boss.m_state = BossState::Dead;
				if (hasVelocity)
					m_componentManager.get<component::VelocityComponent>(entityId).m_velocity = {};
				if (m_componentManager.has<component::AnimationComponent>(entityId))
					m_componentManager.get<component::AnimationComponent>(entityId).m_requested = constant::AnimationState::Dying;
				continue;
			}

			// アニメ・演出ロックを進める
			if (boss.m_animLockTimer > 0.0f)
				boss.m_animLockTimer -= deltaTime;

			// --- フェーズ移行（1回だけ） ---
			if (!boss.m_phase2Triggered && boss.m_config.m_hasPhase2)
			{
				const float hpRatio{ (health.m_maxHp > 0.0f) ? (health.m_currentHp / health.m_maxHp) : 1.0f };
				if (hpRatio <= boss.m_config.m_phase2HpRatio)
				{
					boss.m_phase2Triggered = true;
					boss.m_currentPhase = core::data::BossPhase::Awakened;
					boss.m_state = BossState::PhaseTransition;
					boss.m_animLockTimer = PHASE_TRANSITION_LOCK;
					boss.m_actionTimer = boss.currentPhase().m_actionInterval;
					if (hasVelocity)
						m_componentManager.get<component::VelocityComponent>(entityId).m_velocity = {};
					if (m_componentManager.has<component::AnimationComponent>(entityId))
						m_componentManager.get<component::AnimationComponent>(entityId).m_requested = constant::AnimationState::Idle;

					// 覚醒演出（カメラズーム・シェイク・赤ビネット）のトリガー
					m_eventBus.publish(event::BossPhaseTransitionEvent{ entityId, core::data::BossPhase::Awakened });
					continue;
				}
			}

			// --- ターゲット・距離 ---
			if (!ai.m_isActive || ai.m_targetEntity.getId() == 0)
			{
				boss.m_state = BossState::Idle;
				if (hasVelocity)
					m_componentManager.get<component::VelocityComponent>(entityId).m_velocity = {};
				continue;
			}

			const auto& targetTransform{ m_componentManager.get<component::TransformComponent>(ai.m_targetEntity.getId()) };
			core::Vector3 dir{
				targetTransform.m_position.x - transform.m_position.x,
				0.0f,
				targetTransform.m_position.z - transform.m_position.z
			};
			const float distance{ std::sqrt(dir.x * dir.x + dir.z * dir.z) };
			if (distance > 0.0f)
			{
				dir.x /= distance;
				dir.z /= distance;
				// プレイヤーの方を向く
				transform.m_rotation.y = std::atan2f(-dir.x, -dir.z);
			}

			// 索敵範囲外なら待機
			if (distance > ai.m_detectionRange)
			{
				boss.m_state = BossState::Idle;
				if (hasVelocity)
					m_componentManager.get<component::VelocityComponent>(entityId).m_velocity = {};
				if (m_componentManager.has<component::AnimationComponent>(entityId))
					m_componentManager.get<component::AnimationComponent>(entityId).m_requested = constant::AnimationState::Idle;
				continue;
			}

			// アクション再生中（ロック中）はその場で停止して待つ
			if (boss.m_animLockTimer > 0.0f)
			{
				if (hasVelocity)
					m_componentManager.get<component::VelocityComponent>(entityId).m_velocity = {};
				continue;
			}

			const auto& phase{ boss.currentPhase() };

			// アクション抽選までのカウントダウン
			boss.m_actionTimer -= deltaTime;
			if (boss.m_actionTimer > 0.0f)
			{
				// --- 追跡（近接レンジ外なら接近、レンジ内なら待機） ---
				boss.m_state = BossState::Chase;
				if (distance > phase.m_meleeRange)
				{
					if (hasVelocity)
					{
						auto& velocity{ m_componentManager.get<component::VelocityComponent>(entityId) };
						velocity.m_velocity.x = dir.x * phase.m_moveSpeed;
						velocity.m_velocity.z = dir.z * phase.m_moveSpeed;
					}
					if (m_componentManager.has<component::AnimationComponent>(entityId))
					{
						// Phase2は走り（Run）で覚醒感を出す
						const auto moveAnim{ (boss.m_currentPhase == core::data::BossPhase::Awakened)
							                     ? constant::AnimationState::Run
							                     : constant::AnimationState::Walk };
						m_componentManager.get<component::AnimationComponent>(entityId).m_requested = moveAnim;
					}
				}
				else
				{
					if (hasVelocity)
						m_componentManager.get<component::VelocityComponent>(entityId).m_velocity = {};
					if (m_componentManager.has<component::AnimationComponent>(entityId))
						m_componentManager.get<component::AnimationComponent>(entityId).m_requested = constant::AnimationState::Idle;
				}
				continue;
			}

			// --- アクション抽選＆実行 ---
			if (hasVelocity)
				m_componentManager.get<component::VelocityComponent>(entityId).m_velocity = {};

			const int aliveMinions{ countAliveMinions() };
			const bool canSummon{ !phase.m_summonTypes.empty() && phase.m_summonCount > 0 && aliveMinions < phase.m_summonMax };

			const BossState action{ chooseAction(phase, distance, canSummon) };
			switch (action)
			{
			case BossState::Melee:
				performMelee(entityId);
				boss.m_state = BossState::Melee;
				boss.m_animLockTimer = MELEE_LOCK;
				break;
			case BossState::Ranged:
				performRanged(entityId, transform, dir, phase);
				boss.m_state = BossState::Ranged;
				boss.m_animLockTimer = RANGED_LOCK;
				break;
			case BossState::Summon:
				performSummon(transform, phase, phase.m_summonMax - aliveMinions);
				boss.m_state = BossState::Summon;
				boss.m_animLockTimer = SUMMON_LOCK;
				break;
			default:
				boss.m_state = BossState::Chase; // 候補なし：次フレームで追跡に戻る
				break;
			}
			boss.m_actionTimer = phase.m_actionInterval;
		}
	}

	component::ai::BossState BossAISystem::chooseAction(const core::data::BossPhaseData& phase,
	    float distance, bool canSummon)
	{
		using component::ai::BossState;

		// 近接は近接レンジ内のときだけ候補にする
		const int wMelee{ (distance <= phase.m_meleeRange) ? phase.m_weightMelee : 0 };
		const int wRanged{ phase.m_weightRanged };
		const int wSummon{ canSummon ? phase.m_weightSummon : 0 };
		const int total{ wMelee + wRanged + wSummon };
		if (total <= 0)
			return BossState::Chase;

		std::uniform_int_distribution<int> dist{ 0, total - 1 };
		int roll{ dist(m_rng) };
		if (roll < wMelee)
			return BossState::Melee;
		roll -= wMelee;
		if (roll < wRanged)
			return BossState::Ranged;
		return BossState::Summon;
	}

	void BossAISystem::performMelee(core::ecs::EntityId entityId)
	{
		if (m_componentManager.has<component::AttackComponent>(entityId))
			m_componentManager.get<component::AttackComponent>(entityId).m_attackRequested = true;
		if (m_componentManager.has<component::AnimationComponent>(entityId))
			m_componentManager.get<component::AnimationComponent>(entityId).m_requested = constant::AnimationState::Attack1;
	}

	void BossAISystem::performRanged(core::ecs::EntityId entityId, const component::TransformComponent& transform,
	    const core::Vector3& dirToTarget, const core::data::BossPhaseData& phase)
	{
		const int count{ phase.m_rainbowCount };
		if (count <= 0)
			return;

		const float spreadRad{ phase.m_rainbowSpreadDeg * DEG_TO_RAD };

		for (int i{ 0 }; i < count; ++i)
		{
			// 扇の中央を0として -0.5〜0.5 に割り振り、全開き角ぶん回転させる
			const float t{ (count == 1) ? 0.0f : (static_cast<float>(i) / static_cast<float>(count - 1) - 0.5f) };
			const float angle{ t * spreadRad };
			const float ca{ std::cosf(angle) };
			const float sa{ std::sinf(angle) };

			// dirToTarget（水平・正規化済み）をY軸まわりに回す
			core::Vector3 fanDir{
				dirToTarget.x * ca - dirToTarget.z * sa,
				0.0f,
				dirToTarget.x * sa + dirToTarget.z * ca
			};

			core::Vector3 origin{
				transform.m_position.x + fanDir.x * m_rainbowMeta.m_spawnForward,
				transform.m_position.y + m_rainbowMeta.m_spawnHeight,
				transform.m_position.z + fanDir.z * m_rainbowMeta.m_spawnForward
			};

			factory::ProjectileConfig config{};
			config.m_speed = m_rainbowMeta.m_speed;
			config.m_damage = m_rainbowMeta.m_damage;
			config.m_lifetime = m_rainbowMeta.m_lifetime;
			config.m_radius = m_rainbowRadius;
			config.m_scale = m_rainbowMeta.m_scale;
			config.m_modelHandle = m_rainbowModelHandle;

			m_projectileFactory.spawn(origin, fanDir, config, constant::Tag::Enemy);
		}

		if (m_componentManager.has<component::AnimationComponent>(entityId))
			m_componentManager.get<component::AnimationComponent>(entityId).m_requested = constant::AnimationState::Attack2;
	}

	void BossAISystem::performSummon(const component::TransformComponent& transform,
	    const core::data::BossPhaseData& phase, int summonSlots)
	{
		const int n{ std::min(phase.m_summonCount, summonSlots) };
		if (n <= 0 || phase.m_summonTypes.empty())
			return;

		std::uniform_int_distribution<std::size_t> typeDist{ 0, phase.m_summonTypes.size() - 1 };
		std::uniform_real_distribution<float> angleDist{ 0.0f, 2.0f * PI };
		std::uniform_real_distribution<float> radiusDist{ phase.m_summonRadiusMin, phase.m_summonRadiusMax };

		for (int i{ 0 }; i < n; ++i)
		{
			const auto& typeStr{ phase.m_summonTypes[typeDist(m_rng)] };
			const auto type{ constant::toEnemyType(typeStr) };

			const float angle{ angleDist(m_rng) };
			const float radius{ radiusDist(m_rng) };
			core::Vector3 pos{
				transform.m_position.x + std::cosf(angle) * radius,
				transform.m_position.y,
				transform.m_position.z + std::sinf(angle) * radius
			};

			m_enemySpawner.spawn(type, pos);
		}
	}

	int BossAISystem::countAliveMinions()
	{
		int count{ 0 };
		auto entities{ m_componentManager.getAllEntities<component::AIComponent>() };
		for (auto entityId : entities)
		{
			// ボス自身は雑魚数に含めない
			if (m_componentManager.has<component::ai::BossAIComponent>(entityId))
				continue;
			if (!m_componentManager.has<component::HealthComponent>(entityId))
				continue;
			if (!m_componentManager.get<component::HealthComponent>(entityId).m_isDead)
				++count;
		}
		return count;
	}
} // namespace game::system::ai
