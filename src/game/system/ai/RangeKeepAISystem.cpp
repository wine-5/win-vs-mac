#include "RangeKeepAISystem.h"
#include "game/component/ai/RangeKeepAIComponent.h"
#include "game/component/AIComponent.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/AttackComponent.h"
#include "game/component/AnimationComponent.h"
#include "game/constant/AnimationState.h"
#include <cmath>
#include <algorithm>

namespace game::system::ai
{
	RangeKeepAISystem::RangeKeepAISystem(core::ecs::ComponentManager& componentManager)
	    : m_componentManager{ componentManager }
	{
	}

	void RangeKeepAISystem::update(float deltaTime)
	{
		// RangeKeepAIComponentを持つ敵だけを処理する
		auto entities{ m_componentManager.getAllEntities<component::ai::RangeKeepAIComponent>() };

		for (auto entityId : entities)
		{
			if (!m_componentManager.has<component::AIComponent>(entityId))
				continue;

			auto& ai{ m_componentManager.get<component::AIComponent>(entityId) };
			auto& rangeKeep{ m_componentManager.get<component::ai::RangeKeepAIComponent>(entityId) };

			// AIが無効なら処理をスキップ
			if (!ai.m_isActive)
				continue;

			// 追跡対象が設定されていない場合はスキップ
			if (ai.m_targetEntity.getId() == 0)
				continue;

			auto& transform{ m_componentManager.get<component::TransformComponent>(entityId) };
			auto& targetTransform{ m_componentManager.get<component::TransformComponent>(ai.m_targetEntity.getId()) };

			// ターゲットへの方向ベクトルを計算（3D全軸）
			core::Vector3 direction{};
			direction.x = targetTransform.m_position.x - transform.m_position.x;
			direction.y = targetTransform.m_position.y - transform.m_position.y;
			direction.z = targetTransform.m_position.z - transform.m_position.z;

			float distance{ std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z) };

			// 索敵範囲外なら何もしない
			if (distance > ai.m_detectionRange)
				continue;

			// 方向ベクトルを正規化
			if (distance > 0.0f)
			{
				direction.x /= distance;
				direction.y /= distance;
				direction.z /= distance;
			}

			// 距離維持ロジック：推奨距離の範囲内に入るよう移動する
			core::Vector3 moveDirection{ direction };
			if (distance < rangeKeep.m_preferredDistanceMin)
			{
				// 近すぎたら後退
				moveDirection.x = -direction.x;
				moveDirection.y = -direction.y;
				moveDirection.z = -direction.z;
			}
			else if (distance > rangeKeep.m_preferredDistanceMax)
			{
				// 遠すぎたら接近（そのままdirection）
				// moveDirection = direction; // 既にそう
			}
			else
			{
				// 推奨距離内なら停止
				moveDirection = core::Vector3{ 0.0f, 0.0f, 0.0f };
			}

			// 移動速度を設定（水平と垂直を分ける場合もあるが、ここでは統一）
			if (m_componentManager.has<component::VelocityComponent>(entityId))
			{
				auto& velocity{ m_componentManager.get<component::VelocityComponent>(entityId) };
				velocity.m_velocity.x = moveDirection.x * ai.m_moveSpeed;
				velocity.m_velocity.y = moveDirection.y * ai.m_moveSpeed;
				velocity.m_velocity.z = moveDirection.z * ai.m_moveSpeed;
			}

			// ホバー高度を保つ（浮遊敵用）
			if (rangeKeep.m_hoverHeight > 0.0f)
			{
				// ホバー高度と現在位置の差を垂直速度に反映させる
				// （重力があれば、重力で下がるので、その分を補正）
				const float heightDiff{ rangeKeep.m_hoverHeight - transform.m_position.y };
				const float hoverRestoreSpeed{ 50.0f }; // ホバー高度への復帰速度
				if (m_componentManager.has<component::VelocityComponent>(entityId))
				{
					auto& velocity{ m_componentManager.get<component::VelocityComponent>(entityId) };
					velocity.m_velocity.y += heightDiff * hoverRestoreSpeed * deltaTime;
				}
			}

			// 向きを更新（プレイヤーの方を向く、水平面のみ）
			const float horizontalDistance{ std::sqrt(direction.x * direction.x + direction.z * direction.z) };
			if (horizontalDistance > 0.0f)
			{
				const float normalizedDirX{ direction.x / horizontalDistance };
				const float normalizedDirZ{ direction.z / horizontalDistance };
				transform.m_rotation.y = std::atan2f(-normalizedDirX, -normalizedDirZ);
			}

			// アニメーション要求：移動中は Walk、停止時は Idle
			if (m_componentManager.has<component::AnimationComponent>(entityId))
			{
				auto& anim{ m_componentManager.get<component::AnimationComponent>(entityId) };
				anim.m_requested = (std::sqrt(moveDirection.x * moveDirection.x + moveDirection.z * moveDirection.z) > 0.01f)
				                       ? constant::AnimationState::Walk
				                       : constant::AnimationState::Idle;
			}

			// 攻撃のクールダウンを更新
			if (ai.m_currentAttackCooldown > 0.0f)
				ai.m_currentAttackCooldown -= deltaTime;

			// 攻撃判定：索敵範囲内かつクールダウンが完了なら攻撃（レンジ判定なし）
			if (m_componentManager.has<component::AttackComponent>(entityId))
			{
				auto& attack{ m_componentManager.get<component::AttackComponent>(entityId) };
				if (ai.m_currentAttackCooldown <= 0.0f)
				{
					attack.m_attackRequested = true;
					ai.m_currentAttackCooldown = ai.m_attackCooldown;
				}
			}
		}
	}
} // namespace game::system::ai
