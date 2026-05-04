#include "AttackSystem.h"
#include "game/component/AttackComponent.h"
#include "game/component/HealthComponent.h"
#include "game/component/InputComponent.h"
#include "game/component/TransformComponent.h"
#include "game/component/TagComponent.h"
#include "game/constant/Tag.h"
#include "game/attack/DamageChain.h"
#include "game/attack/BaseAttackHandler.h"
#include "game/attack/DefenseHandler.h"
#include "game/event/InGameEvents.h"
#include <cmath>
#include <core\interface\ILogger.h>

namespace game::system
{
	AttackSystem::AttackSystem(core::ecs::ComponentManager& componentManager, EventBus& eventBus)
		: m_componentManager{ componentManager }
		, m_eventBus{ eventBus }
	{
	}

	void AttackSystem::update(float deltaTime)
	{
		auto attackers{ m_componentManager.getAllEntities<component::AttackComponent>() };

		for (auto attackerId : attackers)
		{
			auto& attack{ m_componentManager.get<component::AttackComponent>(attackerId) };

			// クールダウンを更新
			if (attack.m_currentCooldown > 0.0f)
			{
				attack.m_currentCooldown -= deltaTime;
				continue; // まだ攻撃できないためスキップ
			}

			if (m_componentManager.has<component::InputComponent>(attackerId))
			{

				auto& input{ m_componentManager.get<component::InputComponent>(attackerId) };
				if (input.m_attackPressed)
					attack.m_attackRequested = true;
			}

			if (!attack.m_attackRequested)
				continue;

			attack.m_attackRequested = false;

			auto targets{ m_componentManager.getAllEntities<component::HealthComponent>() };
			for (auto targetId : targets)
			{
				if (targetId == attackerId)
					continue;

				// AttackComponentを持つEntityの攻撃範囲チェック
				auto& attackerTransform{ m_componentManager.get<component::TransformComponent>(attackerId) };
				auto& targetTransform{ m_componentManager.get<component::TransformComponent>(targetId) };
				float dx{ attackerTransform.m_position.x - targetTransform.m_position.x };
				float dz{ attackerTransform.m_position.z - targetTransform.m_position.z };
				float distance{ std::sqrt(dx * dx + dz * dz) };

				if (distance > attack.m_attackRange) // 攻撃範囲外の場合
					continue;

				// CORチェーンでダメージ計算を行う
				auto base{ std::make_unique<attack::BaseAttackHandler>(m_componentManager) };
				auto defense{ std::make_unique<attack::DefenseHandler>(m_componentManager) };
				base->setNext(std::move(defense));

				attack::DamageChain chain{};
				chain.m_attackId = attackerId;
				chain.m_targetId = targetId;

				base->handle(chain);

				// HPを減らす
				auto& health{ m_componentManager.get<component::HealthComponent>(targetId) };
				health.m_currentHp -= chain.m_damage;

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
						m_eventBus.publish(event::EnemyDeadEvent{targetId});
				}

				// AttackHitイベントを発行する
				event::AttackHitEvent hitEvent{};
				hitEvent.m_attackerId = attackerId;
				hitEvent.m_targetId = targetId;
				hitEvent.m_damage = chain.m_damage;
				m_eventBus.publish(hitEvent);
			}

			// クールダウンをリセット
			attack.m_currentCooldown = attack.m_attackCooldown;
		}
	}
}