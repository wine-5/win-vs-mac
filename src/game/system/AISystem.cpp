#include "AISystem.h"
#include "game/component/AIComponent.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/AttackComponent.h"
#include "game/component/AnimationComponent.h"
#include "core/utility/Vector3.h"
#include <cmath>

namespace game::system
{
	AISystem::AISystem(core::ecs::ComponentManager& componentManager)
		: m_componentManager{ componentManager }
	{
	}

	void AISystem::update(float deltaTime)
	{
		// AIComponentを持つ全エンティティを取得
		auto entities{m_componentManager.getAllEntities<component::AIComponent>()};
		for (auto entityId : entities)
		{
			auto& ai = m_componentManager.get<component::AIComponent>(entityId);

			// AIが無効なら処理をスキップ
			if (!ai.m_isActive)
				continue;

			// 追跡対象が設定されていない場合はスキップ
			if(ai.m_targetEntity.getId() == 0)
				continue;

			auto& transform = m_componentManager.get<component::TransformComponent>(entityId);
			auto& targetTransform = m_componentManager.get<component::TransformComponent>(ai.m_targetEntity.getId());

			// ターゲットへの方向ベクトルを計算
			core::Vector3 direction{};
			direction.x = targetTransform.m_position.x - transform.m_position.x;
			direction.y = 0.0f; // Y軸は無視（今後Y軸も追従する可能性あり）
			direction.z = targetTransform.m_position.z - transform.m_position.z;

			float distance{std::sqrt(direction.x * direction.x + direction.z * direction.z)};

			if (distance > ai.m_detectionRange)
				continue;

			if (distance > 0.0f)
			{
				direction.x /= distance;
				direction.z /= distance;
			}

			if (m_componentManager.has<component::VelocityComponent>(entityId))
			{
				auto& velocity = m_componentManager.get<component::VelocityComponent>(entityId);
				velocity.m_velocity.x = direction.x * ai.m_moveSpeed;
				velocity.m_velocity.z = direction.z * ai.m_moveSpeed;
			}

			// 移動状態に応じたアニメーションを要求する
			if (m_componentManager.has<component::AnimationComponent>(entityId))
			{
				auto& anim = m_componentManager.get<component::AnimationComponent>(entityId);
				anim.m_requested = (distance > 0.0f)
					? constant::AnimationState::Walk
					: constant::AnimationState::Idle;
			}

			if (distance > 0.0f)
				transform.m_rotation.y = std::atan2f(-direction.x, -direction.z);

			// 攻撃のクールダウンを更新
			if (ai.m_currentAttackCooldown > 0.0f)
				ai.m_currentAttackCooldown -= deltaTime;

			// クールダウンが0なら、AttackComponentに攻撃要求をセットする
			if (ai.m_currentAttackCooldown <= 0.0f)
			{
				if (m_componentManager.has<component::AttackComponent>(entityId))
				{
					auto& attack{ m_componentManager.get<component::AttackComponent>(entityId) };
					attack.m_attackRequested = true;
					ai.m_currentAttackCooldown = ai.m_attackCooldown;
				}
			}
		}
	}
}