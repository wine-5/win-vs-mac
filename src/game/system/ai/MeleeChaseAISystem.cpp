#include "MeleeChaseAISystem.h"
#include "game/component/ai/MeleeChaseAIComponent.h"
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
			if (!m_componentManager.has<component::AIComponent>(entityId))
				continue;

			auto& ai{ m_componentManager.get<component::AIComponent>(entityId) };

			// AIが無効なら処理をスキップ
			if (!ai.m_isActive)
				continue;

			// 追跡対象が設定されていない場合はスキップ
			if (ai.m_targetEntity.getId() == 0)
				continue;

			auto& transform{ m_componentManager.get<component::TransformComponent>(entityId) };
			auto& targetTransform{ m_componentManager.get<component::TransformComponent>(ai.m_targetEntity.getId()) };

			// ターゲットへの方向ベクトルを計算（水平面のみ）
			core::Vector3 direction{};
			direction.x = targetTransform.m_position.x - transform.m_position.x;
			direction.y = 0.0f;
			direction.z = targetTransform.m_position.z - transform.m_position.z;

			float distance{ std::sqrt(direction.x * direction.x + direction.z * direction.z) };

			// 索敵範囲外なら何もしない
			if (distance > ai.m_detectionRange)
				continue;

			// 方向ベクトルを正規化
			if (distance > 0.0f)
			{
				direction.x /= distance;
				direction.z /= distance;
			}

			// 移動速度を設定（AIComponentから移動速度を読む）
			if (m_componentManager.has<component::VelocityComponent>(entityId))
			{
				auto& velocity{ m_componentManager.get<component::VelocityComponent>(entityId) };
				velocity.m_velocity.x = direction.x * ai.m_moveSpeed;
				velocity.m_velocity.z = direction.z * ai.m_moveSpeed;
			}

			// 向きを更新（プレイヤーの方を向く）
			if (distance > 0.0f)
				transform.m_rotation.y = std::atan2f(-direction.x, -direction.z);

			// アニメーション要求：移動中は Walk、停止時は Idle
			if (m_componentManager.has<component::AnimationComponent>(entityId))
			{
				auto& anim{ m_componentManager.get<component::AnimationComponent>(entityId) };
				anim.m_requested = (distance > 0.0f)
				                       ? constant::AnimationState::Walk
				                       : constant::AnimationState::Idle;
			}

			// 攻撃のクールダウンを更新
			if (ai.m_currentAttackCooldown > 0.0f)
				ai.m_currentAttackCooldown -= deltaTime;

			// 攻撃判定：レンジ内かつクールダウンが完了なら攻撃
			if (m_componentManager.has<component::AttackComponent>(entityId))
			{
				auto& attack{ m_componentManager.get<component::AttackComponent>(entityId) };
				float attackRange{ attack.m_attackRange };

				// レンジ内かつクールダウンが0以下なら攻撃要求
				if (distance <= attackRange && ai.m_currentAttackCooldown <= 0.0f)
				{
					attack.m_attackRequested = true;
					// Attack1アニメを要求
					if (m_componentManager.has<component::AnimationComponent>(entityId))
					{
						auto& anim{ m_componentManager.get<component::AnimationComponent>(entityId) };
						anim.m_requested = constant::AnimationState::Attack1;
					}
					ai.m_currentAttackCooldown = ai.m_attackCooldown;
				}
			}
		}
	}
} // namespace game::system::ai
