#include "MeleeChaseAISystem.h"
#include "game/component/ai/MeleeChaseAIComponent.h"
#include "game/component/ai/PatrolComponent.h"
#include "game/component/ai/AIComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/visual/AnimationComponent.h"
#include "game/constant/AnimationState.h"
#include "game/utility/GroundQuery.h"
#include <cmath>
#include <algorithm>
#include "core/utility/MathConstants.h"

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
	// 崖チェックで足元を確かめる距離（進行方向へこのぶん先を見る）。
	// 短すぎると止まりきれず、長すぎると通れる細道まで避けてしまう
	constexpr float EDGE_PROBE_DISTANCE{ 70.0f };
	// 踏み出してよい下りの落差。段差はそのまま降りるが、これを超える崖には向かわない
	constexpr float MAX_STEP_DOWN{ 150.0f };
	// 崖チェックで見上げる高さ（GroundingSystemの段差許容と揃える）
	constexpr float STEP_UP_TOLERANCE{ 40.0f };
	// 徘徊の目的地を選び直す最大回数。床の上を引けなければ諦めてスポーン地点へ戻す
	constexpr int WANDER_PICK_ATTEMPTS{ 8 };
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

		// 攻撃モーション中はレンジから外れても足を止める。
		// 溜め・振りの最中に追いかけると滑って見え、間合いを外して避ける動きも成立しなくなる
		const bool isAttacking{ isAttackInProgress(entityId) };

		// プレイヤーが崖の向こうにいても、追いかけて落ちないよう足元で止まる
		const bool atEdge{ !canStepToward(entityId, dirToPlayer) };

		// 移動：攻撃レンジ内・攻撃モーション中・崖の縁では止まり、それ以外なら接近する
		// （従来はレンジ内でも速度を与え続け、プレイヤーへ押し込んでいた）
		if (inAttackRange || isAttacking || atEdge)
			stopHorizontalMovement(entityId);
		else if (m_componentManager.has<component::movement::VelocityComponent>(entityId))
		{
			auto& velocity{ m_componentManager.get<component::movement::VelocityComponent>(entityId) };
			velocity.m_velocity.x = dirToPlayer.x * ai.m_moveSpeed;
			velocity.m_velocity.z = dirToPlayer.z * ai.m_moveSpeed;
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

		// アニメ要求：攻撃時はAttack1、レンジ内待機はIdle、接近中はWalk。
		// 攻撃モーションの再生中は歩き・待機で上書きせず、最後まで振らせる
		if (attacking)
			requestAnimation(entityId, constant::AnimationState::Attack1);
		else if (isAttacking)
			return;
		else if (inAttackRange || atEdge)
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

		// 攻撃モーション中に索敵から外れて巡回へ移った場合も、振り終わるまでは動かさない
		if (isAttackInProgress(entityId))
		{
			stopHorizontalMovement(entityId);
			return;
		}

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

		toTarget.x /= distance;
		toTarget.z /= distance;

		// 目的地との間に崖があるなら、その手前で立ち止まって別の目的地を選び直す。
		// 目的地自体は床の上でも、そこへ向かう直線が奈落をまたぐことはある
		if (!canStepToward(entityId, toTarget))
		{
			patrol.m_hasWanderTarget = false;
			std::uniform_real_distribution<float> pauseDist{ PAUSE_MIN, PAUSE_MAX };
			patrol.m_pauseTimer = pauseDist(m_rng);
			stopHorizontalMovement(entityId);
			requestAnimation(entityId, constant::AnimationState::Idle);
			return;
		}

		// 目的地へゆっくり移動し、その方向を向く
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
		std::uniform_real_distribution<float> angleDist{ 0.0f, core::utility::TWO_PI };
		std::uniform_real_distribution<float> radiusDist{ WANDER_RADIUS_MIN, WANDER_RADIUS_MAX };

		// 床の無い場所を目的地にすると、そこを目指して崖から歩き出してしまう。
		// 引き直しても床の上を引けなければスポーン地点へ戻す（そこは必ず床の上）
		for (int attempt{ 0 }; attempt < WANDER_PICK_ATTEMPTS; ++attempt)
		{
			const float angle{ angleDist(m_rng) };
			const float radius{ radiusDist(m_rng) };

			core::Vector3 target{ home };
			target.x += std::cos(angle) * radius;
			target.z += std::sin(angle) * radius;

			if (utility::findGround(m_componentManager, target.x, target.z, home.y + STEP_UP_TOLERANCE).has_value())
				return target;
		}
		return home;
	}

	bool MeleeChaseAISystem::canStepToward(core::ecs::EntityId entityId, const core::Vector3& direction) const
	{
		// 空中にいる間は判定しない。落下や吹き飛びの最中に足を止めても意味が無く、
		// 崖から落ちた敵が空中で固まって見えるだけになる
		const auto* velocity{ m_componentManager.tryGet<component::movement::VelocityComponent>(entityId) };
		if (velocity == nullptr || !velocity->m_isGrounded)
			return true;

		const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(entityId) };
		const float foot{ transform.m_position.y };
		const float probeX{ transform.m_position.x + direction.x * EDGE_PROBE_DISTANCE };
		const float probeZ{ transform.m_position.z + direction.z * EDGE_PROBE_DISTANCE };

		const auto ground{ utility::findGround(m_componentManager, probeX, probeZ, foot + STEP_UP_TOLERANCE) };
		if (!ground.has_value())
			return false; // その先は奈落

		// 段差程度なら降りてよい。それより深ければ崖とみなす
		return foot - ground->m_height <= MAX_STEP_DOWN;
	}

	bool MeleeChaseAISystem::isAttackInProgress(core::ecs::EntityId entityId) const
	{
		// 溜め中はまだ振っていないが、すでに攻撃に入っているので動かさない
		const auto* attack{ m_componentManager.tryGet<component::combat::AttackComponent>(entityId) };
		if (attack != nullptr && attack->m_windupPending)
			return true;

		// 振りの最中：攻撃アニメが再生中で、まだ終端に達していない
		const auto* anim{ m_componentManager.tryGet<component::visual::AnimationComponent>(entityId) };
		return anim != nullptr &&
		       anim->m_current == constant::AnimationState::Attack1 &&
		       !anim->m_isCompleted;
	}

	void MeleeChaseAISystem::stopHorizontalMovement(core::ecs::EntityId entityId)
	{
		auto* velocity{ m_componentManager.tryGet<component::movement::VelocityComponent>(entityId) };
		if (velocity == nullptr)
			return;

		// 落下は止めない。Yを触ると空中で固まってしまう
		velocity->m_velocity.x = 0.0f;
		velocity->m_velocity.z = 0.0f;
	}

	void MeleeChaseAISystem::requestAnimation(core::ecs::EntityId entityId, constant::AnimationState state)
	{
		if (m_componentManager.has<component::visual::AnimationComponent>(entityId))
			m_componentManager.get<component::visual::AnimationComponent>(entityId).m_requested = state;
	}
} // namespace game::system::ai
