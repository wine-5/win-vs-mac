#include "MeleeChaseAISystem.h"
#include "game/component/ai/MeleeChaseAIComponent.h"
#include "game/component/ai/PatrolComponent.h"
#include "game/component/ai/AIComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/visual/AnimationComponent.h"
#include "game/constant/AnimationState.h"
#include <cmath>
#include <algorithm>
#include <numbers>

namespace
{
	// 巡回時の移動速度倍率（追跡時より遅くうろつかせる）
	constexpr float PATROL_SPEED_FACTOR{ 0.7f };
	// 徘徊目的地をスポーン地点から選ぶ距離の範囲
	constexpr float WANDER_RADIUS_MIN{ 40.0f };
	constexpr float WANDER_RADIUS_MAX{ 160.0f };
	// 目的地に到着したとみなす距離
	constexpr float WANDER_REACH_DISTANCE{ 15.0f };
	// 目的地到着後に立ち止まる時間の範囲（秒）
	constexpr float PAUSE_MIN{ 1.0f };
	constexpr float PAUSE_MAX{ 2.5f };
} // namespace

namespace game::system::ai
{
	MeleeChaseAISystem::MeleeChaseAISystem(core::ecs::ComponentManager& componentManager)
	    : m_componentManager{ componentManager }
	{
	}

	void MeleeChaseAISystem::update(float deltaTime)
	{
		// MeleeChaseAIComponentを持つ敵だけを処理する
		auto entities{ m_componentManager.getAllEntities<component::ai::MeleeChaseAIComponent>() };

		for (auto entityId : entities)
		{
			if (!m_componentManager.has<component::ai::AIComponent>(entityId))
				continue;

			auto& ai{ m_componentManager.get<component::ai::AIComponent>(entityId) };

			// AIが無効なら処理をスキップ（死亡後など）
			if (!ai.m_isActive)
				continue;

			auto& melee{ m_componentManager.get<component::ai::MeleeChaseAIComponent>(entityId) };
			auto& patrol{ m_componentManager.get<component::ai::PatrolComponent>(entityId) };
			auto& transform{ m_componentManager.get<component::movement::TransformComponent>(entityId) };

			// 徘徊の基準点（スポーン地点）を初回だけ記録する
			if (!patrol.m_homeInitialized)
			{
				patrol.m_homePosition = transform.m_position;
				patrol.m_homeInitialized = true;
			}

			// プレイヤーとの水平距離・方向を測り、索敵範囲内かどうかで状態を切り替える
			bool canSeePlayer{ false };
			core::Vector3 dirToPlayer{};
			float distanceToPlayer{ 0.0f };
			if (ai.m_targetEntity.getId() != 0)
			{
				auto& targetTransform{ m_componentManager.get<component::movement::TransformComponent>(ai.m_targetEntity.getId()) };
				dirToPlayer.x = targetTransform.m_position.x - transform.m_position.x;
				dirToPlayer.z = targetTransform.m_position.z - transform.m_position.z;
				distanceToPlayer = std::sqrt(dirToPlayer.x * dirToPlayer.x + dirToPlayer.z * dirToPlayer.z);
				if (distanceToPlayer > 0.0f)
				{
					dirToPlayer.x /= distanceToPlayer;
					dirToPlayer.z /= distanceToPlayer;
				}
				canSeePlayer = distanceToPlayer <= ai.m_detectionRange;
			}

			melee.m_state = canSeePlayer ? component::ai::MeleeChaseState::Chase
			                             : component::ai::MeleeChaseState::Patrol;

			if (melee.m_state == component::ai::MeleeChaseState::Chase)
				updateChase(entityId, distanceToPlayer, dirToPlayer, deltaTime);
			else
				updatePatrol(entityId, deltaTime);
		}
	}

	void MeleeChaseAISystem::updateChase(core::ecs::EntityId entityId, float distanceToPlayer,
	    const core::Vector3& dirToPlayer, float deltaTime)
	{
		auto& ai{ m_componentManager.get<component::ai::AIComponent>(entityId) };
		auto& transform{ m_componentManager.get<component::movement::TransformComponent>(entityId) };

		// 攻撃レンジ内かどうかを判定
		bool inAttackRange{ false };
		if (m_componentManager.has<component::combat::AttackComponent>(entityId))
			inAttackRange = distanceToPlayer <= m_componentManager.get<component::combat::AttackComponent>(entityId).m_attackRange;

		// 移動：攻撃レンジ内では止まり、外なら接近する
		// （従来はレンジ内でも速度を与え続け、プレイヤーへ押し込んでいた）
		if (m_componentManager.has<component::movement::VelocityComponent>(entityId))
		{
			auto& velocity{ m_componentManager.get<component::movement::VelocityComponent>(entityId) };
			if (inAttackRange)
			{
				velocity.m_velocity.x = 0.0f;
				velocity.m_velocity.z = 0.0f;
			}
			else
			{
				velocity.m_velocity.x = dirToPlayer.x * ai.m_moveSpeed;
				velocity.m_velocity.z = dirToPlayer.z * ai.m_moveSpeed;
			}
		}

		// 常にプレイヤーの方を向く
		if (distanceToPlayer > 0.0f)
			transform.m_rotation.y = std::atan2f(-dirToPlayer.x, -dirToPlayer.z);

		// 攻撃：レンジ内なら毎フレーム要求だけ出す。
		// 実際に撃つ間隔はAttackComponentのクールダウンでAttackSystemが管理する
		bool attacking{ false };
		if (auto* attack{ m_componentManager.tryGet<component::combat::AttackComponent>(entityId) })
		{
			if (inAttackRange)
				attack->m_attackRequested = true;

			// 攻撃アニメはAttackSystemが実際に攻撃を開始したフレームだけ要求する
			attacking = attack->m_justFired;
		}

		// アニメ要求：攻撃時はAttack1、レンジ内待機はIdle、接近中はWalk
		if (attacking)
			requestAnimation(entityId, constant::AnimationState::Attack1);
		else if (inAttackRange)
			requestAnimation(entityId, constant::AnimationState::Idle);
		else
			requestAnimation(entityId, constant::AnimationState::Walk);
	}

	void MeleeChaseAISystem::updatePatrol(core::ecs::EntityId entityId, float deltaTime)
	{
		auto& ai{ m_componentManager.get<component::ai::AIComponent>(entityId) };
		auto& patrol{ m_componentManager.get<component::ai::PatrolComponent>(entityId) };
		auto& transform{ m_componentManager.get<component::movement::TransformComponent>(entityId) };

		const bool hasVelocity{ m_componentManager.has<component::movement::VelocityComponent>(entityId) };

		// 立ち止まり中：時間を消化し、その間は停止＋Idle
		if (patrol.m_pauseTimer > 0.0f)
		{
			patrol.m_pauseTimer -= deltaTime;
			if (hasVelocity)
			{
				auto& velocity{ m_componentManager.get<component::movement::VelocityComponent>(entityId) };
				velocity.m_velocity.x = 0.0f;
				velocity.m_velocity.z = 0.0f;
			}
			requestAnimation(entityId, constant::AnimationState::Idle);
			return;
		}

		// 目的地が無ければスポーン地点まわりから新たに選ぶ
		if (!patrol.m_hasWanderTarget)
		{
			patrol.m_wanderTarget = pickWanderTarget(patrol.m_homePosition);
			patrol.m_hasWanderTarget = true;
		}

		// 目的地への水平距離・方向
		core::Vector3 toTarget{};
		toTarget.x = patrol.m_wanderTarget.x - transform.m_position.x;
		toTarget.z = patrol.m_wanderTarget.z - transform.m_position.z;
		const float distance{ std::sqrt(toTarget.x * toTarget.x + toTarget.z * toTarget.z) };

		// 到着したら停止して少し立ち止まり、次のフレーム以降で新たな目的地を選ぶ
		if (distance <= WANDER_REACH_DISTANCE)
		{
			patrol.m_hasWanderTarget = false;
			std::uniform_real_distribution<float> pauseDist{ PAUSE_MIN, PAUSE_MAX };
			patrol.m_pauseTimer = pauseDist(m_rng);
			if (hasVelocity)
			{
				auto& velocity{ m_componentManager.get<component::movement::VelocityComponent>(entityId) };
				velocity.m_velocity.x = 0.0f;
				velocity.m_velocity.z = 0.0f;
			}
			requestAnimation(entityId, constant::AnimationState::Idle);
			return;
		}

		// 目的地へゆっくり移動し、その方向を向く
		toTarget.x /= distance;
		toTarget.z /= distance;
		const float patrolSpeed{ ai.m_moveSpeed * PATROL_SPEED_FACTOR };
		if (hasVelocity)
		{
			auto& velocity{ m_componentManager.get<component::movement::VelocityComponent>(entityId) };
			velocity.m_velocity.x = toTarget.x * patrolSpeed;
			velocity.m_velocity.z = toTarget.z * patrolSpeed;
		}
		transform.m_rotation.y = std::atan2f(-toTarget.x, -toTarget.z);
		requestAnimation(entityId, constant::AnimationState::Walk);
	}

	core::Vector3 MeleeChaseAISystem::pickWanderTarget(const core::Vector3& home)
	{
		std::uniform_real_distribution<float> angleDist{ 0.0f, 2.0f * std::numbers::pi_v<float> };
		std::uniform_real_distribution<float> radiusDist{ WANDER_RADIUS_MIN, WANDER_RADIUS_MAX };
		const float angle{ angleDist(m_rng) };
		const float radius{ radiusDist(m_rng) };

		core::Vector3 target{ home };
		target.x += std::cos(angle) * radius;
		target.z += std::sin(angle) * radius;
		return target;
	}

	void MeleeChaseAISystem::requestAnimation(core::ecs::EntityId entityId, constant::AnimationState state)
	{
		if (m_componentManager.has<component::visual::AnimationComponent>(entityId))
			m_componentManager.get<component::visual::AnimationComponent>(entityId).m_requested = state;
	}
} // namespace game::system::ai
