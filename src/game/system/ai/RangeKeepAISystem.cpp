#include "RangeKeepAISystem.h"
#include "game/component/ai/RangeKeepAIComponent.h"
#include "game/component/ai/PatrolComponent.h"
#include "game/component/AIComponent.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/AnimationComponent.h"
#include "game/constant/AnimationState.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <numbers>

namespace
{
	constexpr float HOVER_RESTORE_SPEED{ 50.0f }; // ホバー高度への復帰速度
	constexpr float STRAFE_RATIO{ 0.85f };        // 横ストレイフ成分の強さ（前後方向の速度に対する倍率）

	// 索敵範囲外の徘徊（Patrol）パラメータ。近接敵（MeleeChaseAISystem）と同じ考え方で少しゆっくり動かす
	constexpr float PATROL_SPEED_FACTOR{ 0.6f };    // 徘徊時の移動速度倍率（追跡時より遅くふらつかせる）
	constexpr float WANDER_RADIUS_MIN{ 150.0f };    // 徘徊目的地をホームから選ぶ距離（最小）
	constexpr float WANDER_RADIUS_MAX{ 500.0f };    // 徘徊目的地をホームから選ぶ距離（最大）
	constexpr float WANDER_REACH_DISTANCE{ 40.0f }; // 目的地に到着したとみなす水平距離
	constexpr float WANDER_PAUSE_MIN{ 1.0f };       // 到着後にホバー待機する時間（最小・秒）
	constexpr float WANDER_PAUSE_MAX{ 3.0f };       // 到着後にホバー待機する時間（最大・秒）

	// アイドル時のホバー揺らぎ（上下動）：止まっていても「生きている」感を出す
	constexpr float SWAY_AMPLITUDE{ 18.0f }; // 上下に揺れる振幅（ワールド単位）
	constexpr float SWAY_FREQ{ 2.2f };       // 揺れの速さ（rad/秒）
	// 発射時のリコイル：撃った直後に軽く上へ跳ね上がって戻る
	constexpr float RECOIL_KICK{ 90.0f }; // リコイルの押し上げ量係数（m_attackAnimTimer[秒]に掛ける）

	/**
	 * @brief ホバーの目標高度に、アイドル揺らぎと発射リコイルを重ねた値を返す
	 * @param baseHeight 基準のホバー高度
	 * @param elapsedTime 揺らぎの位相計算用の経過時間
	 * @param phase 個体ごとの位相オフセット（群れが同期して揺れないようにする）
	 * @param attackAnimTimer 発射直後の演出タイマー（>0の間リコイルで持ち上がる）
	 * @return 揺らぎ・リコイルを加味した目標高度
	 */
	float hoverTargetHeight(float baseHeight, float elapsedTime, float phase, float attackAnimTimer)
	{
		const float sway{ SWAY_AMPLITUDE * std::sinf(elapsedTime * SWAY_FREQ + phase) };
		const float recoil{ (attackAnimTimer > 0.0f) ? attackAnimTimer * RECOIL_KICK : 0.0f };
		return baseHeight + sway + recoil;
	}
} // namespace

namespace game::system::ai
{
	RangeKeepAISystem::RangeKeepAISystem(core::ecs::ComponentManager& componentManager)
	    : m_componentManager{ componentManager }
	{
	}

	void RangeKeepAISystem::update(float deltaTime)
	{
		m_elapsedTime += deltaTime; // ホバー揺らぎの位相計算に使う

		// RangeKeepAIComponentを持つ敵だけを処理する
		auto entities{ m_componentManager.getAllEntities<component::ai::RangeKeepAIComponent>() };

		for (auto entityId : entities)
		{
			if (!m_componentManager.has<component::AIComponent>(entityId))
				continue;

			// 個体ごとに揺らぎの位相をずらし、複数体が同じタイミングで上下しないようにする
			const float swayPhase{ static_cast<float>(entityId) * 1.7f };

			auto& ai{ m_componentManager.get<component::AIComponent>(entityId) };
			auto& rangeKeep{ m_componentManager.get<component::ai::RangeKeepAIComponent>(entityId) };

			// AIが無効なら処理をスキップ
			if (!ai.m_isActive)
				continue;

			// 追跡対象が設定されていない場合はスキップ
			if (ai.m_targetEntity.getId() == 0)
				continue;

			auto& transform{ m_componentManager.get<component::TransformComponent>(entityId) };

			// 徘徊の基準点（スポーン地点）を初回だけ記録する（共通のPatrolComponentが保持する）
			auto& patrol{ m_componentManager.get<component::ai::PatrolComponent>(entityId) };
			if (!patrol.m_homeInitialized)
			{
				patrol.m_homePosition = transform.m_position;
				patrol.m_homeInitialized = true;
			}

			auto& targetTransform{ m_componentManager.get<component::TransformComponent>(ai.m_targetEntity.getId()) };

			// ターゲットへの方向ベクトルを計算（3D全軸）
			core::Vector3 direction{};
			direction.x = targetTransform.m_position.x - transform.m_position.x;
			direction.y = targetTransform.m_position.y - transform.m_position.y;
			direction.z = targetTransform.m_position.z - transform.m_position.z;

			float distance{ std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z) };

			// 索敵範囲外なら攻撃・追跡はせず、ホーム周辺をふらつく徘徊に切り替える（その場で固まらせない）
			if (distance > ai.m_detectionRange)
			{
				updatePatrol(entityId, rangeKeep, transform, deltaTime);
				continue;
			}

			// 方向ベクトルを正規化
			if (distance > 0.0f)
			{
				direction.x /= distance;
				direction.y /= distance;
				direction.z /= distance;
			}

			// 距離維持ロジック：推奨距離の帯に入るよう前後方向（ラジアル成分）を決める
			core::Vector3 moveDirection{ 0.0f, 0.0f, 0.0f };
			if (distance < rangeKeep.m_preferredDistanceMin)
			{
				// 近すぎたら後退
				moveDirection.x = -direction.x;
				moveDirection.y = -direction.y;
				moveDirection.z = -direction.z;
			}
			else if (distance > rangeKeep.m_preferredDistanceMax)
			{
				// 遠すぎたら接近
				moveDirection.x = direction.x;
				moveDirection.y = direction.y;
				moveDirection.z = direction.z;
			}
			// 推奨距離帯の中では前後成分は0のまま（横のストレイフのみで周回する）

			// 横ストレイフ成分を合成する：プレイヤーを中心に円周を横滑りして回り込む。
			// 水平方向のdirectionを90°回した横ベクトルに、個体ごとの周回向き(+1/-1)を掛ける。
			// 前後（ラジアル）と横（ストレイフ）を合成することで、間合いを保ちつつ回り込む動きになる。
			const float horizontalLen{ std::sqrt(direction.x * direction.x + direction.z * direction.z) };
			if (horizontalLen > 0.0f)
			{
				const float normalizedX{ direction.x / horizontalLen };
				const float normalizedZ{ direction.z / horizontalLen };
				const float strafeSign{ static_cast<float>(rangeKeep.m_strafeDirection) };
				moveDirection.x += -normalizedZ * strafeSign * STRAFE_RATIO;
				moveDirection.z += normalizedX * strafeSign * STRAFE_RATIO;
			}

			// 移動速度を設定（水平と垂直を分ける場合もあるが、ここでは統一）
			if (m_componentManager.has<component::VelocityComponent>(entityId))
			{
				auto& velocity{ m_componentManager.get<component::VelocityComponent>(entityId) };
				velocity.m_velocity.x = moveDirection.x * ai.m_moveSpeed;
				velocity.m_velocity.y = moveDirection.y * ai.m_moveSpeed;
				velocity.m_velocity.z = moveDirection.z * ai.m_moveSpeed;
			}

			// ホバー高度を保つ（浮遊敵用）。基準高度にアイドル揺らぎ＋発射リコイルを重ねる
			if (rangeKeep.m_hoverHeight > 0.0f)
			{
				// 目標高度（揺らぎ・リコイル込み）と現在位置の差を垂直速度に反映させる
				// （重力があれば、重力で下がるので、その分を補正）
				const float target{ hoverTargetHeight(rangeKeep.m_hoverHeight, m_elapsedTime, swayPhase, rangeKeep.m_attackAnimTimer) };
				const float heightDiff{ target - transform.m_position.y };
				if (m_componentManager.has<component::VelocityComponent>(entityId))
				{
					auto& velocity{ m_componentManager.get<component::VelocityComponent>(entityId) };
					velocity.m_velocity.y += heightDiff * HOVER_RESTORE_SPEED * deltaTime;
				}
			}

			// 向きを更新（プレイヤーの方を向く、水平面のみ）。
			// 機体モデルの正面軸のズレは facingYawOffset で補正する
			const float horizontalDistance{ std::sqrt(direction.x * direction.x + direction.z * direction.z) };
			if (horizontalDistance > 0.0f)
			{
				const float normalizedDirX{ direction.x / horizontalDistance };
				const float normalizedDirZ{ direction.z / horizontalDistance };
				transform.m_rotation.y = std::atan2f(-normalizedDirX, -normalizedDirZ) + rangeKeep.m_facingYawOffset;
			}

			// アニメーション要求：移動中は Walk、停止時は Idle
			if (m_componentManager.has<component::AnimationComponent>(entityId))
			{
				auto& anim{ m_componentManager.get<component::AnimationComponent>(entityId) };
				anim.m_requested = (std::sqrt(moveDirection.x * moveDirection.x + moveDirection.z * moveDirection.z) > 0.01f)
				                       ? constant::AnimationState::Walk
				                       : constant::AnimationState::Idle;
			}

			// 攻撃判定：索敵範囲内なら毎フレーム要求だけ出す（レンジ判定なし）。
			// 実際に撃つ間隔はAttackComponentのクールダウンでAttackSystemが管理する
			if (auto* attack{ m_componentManager.tryGet<component::combat::AttackComponent>(entityId) })
				attack->m_attackRequested = true;
		}
	}
	void RangeKeepAISystem::updatePatrol(core::ecs::EntityId entityId, component::ai::RangeKeepAIComponent& rangeKeep,
	    component::TransformComponent& transform, float deltaTime)
	{
		auto& patrol{ m_componentManager.get<component::ai::PatrolComponent>(entityId) };
		const bool hasVelocity{ m_componentManager.has<component::VelocityComponent>(entityId) };
		const float swayPhase{ static_cast<float>(entityId) * 1.7f };

		// ホバー高度（アイドル揺らぎ込み）を垂直速度へ反映する。徘徊の移動中も待機中も浮遊は保つ
		auto applyHover{ [&](component::VelocityComponent& velocity)
			{
			    if (rangeKeep.m_hoverHeight > 0.0f)
			    {
				    const float target{ hoverTargetHeight(rangeKeep.m_hoverHeight, m_elapsedTime, swayPhase, rangeKeep.m_attackAnimTimer) };
				    velocity.m_velocity.y = (target - transform.m_position.y) * HOVER_RESTORE_SPEED * deltaTime;
			    }
			    else
				    velocity.m_velocity.y = 0.0f;
			} };

		if (!hasVelocity)
			return;
		auto& velocity{ m_componentManager.get<component::VelocityComponent>(entityId) };

		// 到着後の待機中：水平は止めつつホバーは維持する
		if (patrol.m_pauseTimer > 0.0f)
		{
			patrol.m_pauseTimer -= deltaTime;
			velocity.m_velocity.x = 0.0f;
			velocity.m_velocity.z = 0.0f;
			applyHover(velocity);
			return;
		}

		// 目的地が無ければホーム周辺から新たに選ぶ
		if (!patrol.m_hasWanderTarget)
		{
			patrol.m_wanderTarget = pickWanderTarget(patrol.m_homePosition);
			patrol.m_hasWanderTarget = true;
		}

		// 目的地への水平距離・方向
		core::Vector3 toTarget{
			patrol.m_wanderTarget.x - transform.m_position.x,
			0.0f,
			patrol.m_wanderTarget.z - transform.m_position.z
		};
		const float distance{ std::sqrt(toTarget.x * toTarget.x + toTarget.z * toTarget.z) };

		// 到着したら止まって少しホバー待機し、次回に新たな目的地を選ぶ
		if (distance <= WANDER_REACH_DISTANCE)
		{
			patrol.m_hasWanderTarget = false;
			std::uniform_real_distribution<float> pauseDist{ WANDER_PAUSE_MIN, WANDER_PAUSE_MAX };
			patrol.m_pauseTimer = pauseDist(m_rng);
			velocity.m_velocity.x = 0.0f;
			velocity.m_velocity.z = 0.0f;
			applyHover(velocity);
			return;
		}

		// 目的地へゆっくり移動し、その方向を向く（機体正面軸のズレは facingYawOffset で補正）
		const float normalizedX{ toTarget.x / distance };
		const float normalizedZ{ toTarget.z / distance };
		const float patrolSpeed{ m_componentManager.get<component::AIComponent>(entityId).m_moveSpeed * PATROL_SPEED_FACTOR };
		velocity.m_velocity.x = normalizedX * patrolSpeed;
		velocity.m_velocity.z = normalizedZ * patrolSpeed;
		applyHover(velocity);
		transform.m_rotation.y = std::atan2f(-normalizedX, -normalizedZ) + rangeKeep.m_facingYawOffset;

		// アニメを持つ距離維持型が将来出た場合に備えて移動中はWalkを要求する（Safariはアニメ無しで無害）
		if (m_componentManager.has<component::AnimationComponent>(entityId))
			m_componentManager.get<component::AnimationComponent>(entityId).m_requested = constant::AnimationState::Walk;
	}

	core::Vector3 RangeKeepAISystem::pickWanderTarget(const core::Vector3& home)
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
} // namespace game::system::ai
