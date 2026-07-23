#include "CollisionSystem.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/TagComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/combat/DeathComponent.h"
#include "game/constant/Tag.h"
#include <cmath>

namespace
{
	// 死亡中の敵が地面で反発する際の反発係数（1回のバウンドで垂直速度がこの割合になる）
	constexpr float DEATH_BOUNCE_RESTITUTION{ 0.5f };
	// これより落下速度が遅くなったらバウンドをやめて地面で静止させる
	constexpr float DEATH_BOUNCE_MIN_SPEED{ 20.0f };
} // namespace

namespace game::system::combat
{
	CollisionSystem::CollisionSystem(core::ecs::ComponentManager& componentManager)
	    : m_componentManager{ componentManager }
	{
	}

	void CollisionSystem::update(float deltaTime)
	{
		auto entities{ m_componentManager.getAllEntities<component::combat::ColliderComponent>() };

		for (size_t i = 0; i < entities.size(); i++)
		{
			for (size_t j = i + 1; j < entities.size(); j++)
			{
				if (isColliding(entities[i], entities[j]))
					resolveCollision(entities[i], entities[j]);
			}
		}
	}

	bool CollisionSystem::isColliding(core::ecs::EntityId a, core::ecs::EntityId b) const
	{
		auto& transformA = m_componentManager.get<component::movement::TransformComponent>(a);
		auto& transformB = m_componentManager.get<component::movement::TransformComponent>(b);
		auto& colliderA = m_componentManager.get<component::combat::ColliderComponent>(a);
		auto& colliderB = m_componentManager.get<component::combat::ColliderComponent>(b);

		// 各軸の中心座標
		core::Vector3 centerA{ transformA.m_position + colliderA.m_offset };
		core::Vector3 centerB{ transformB.m_position + colliderB.m_offset };

		// 各軸の距離と必要な距離
		float distX{ std::abs(centerA.x - centerB.x) };
		float distY{ std::abs(centerA.y - centerB.y) };
		float distZ{ std::abs(centerA.z - centerB.z) };

		float requiredX{ (colliderA.m_size.x + colliderB.m_size.x) / 2.0f };
		float requiredY{ (colliderA.m_size.y + colliderB.m_size.y) / 2.0f };
		float requiredZ{ (colliderA.m_size.z + colliderB.m_size.z) / 2.0f };

		// 各軸の重なりをチェック
		bool overlapX{ distX <= requiredX };
		bool overlapY{ distY <= requiredY };
		bool overlapZ{ distZ <= requiredZ };

		return overlapX && overlapY && overlapZ;
	}

	void CollisionSystem::resolveCollision(core::ecs::EntityId a, core::ecs::EntityId b)
	{
		const auto& tagA{ m_componentManager.get<component::TagComponent>(a) };
		const auto& tagB{ m_componentManager.get<component::TagComponent>(b) };

		// 地面に乗る側（Player / Enemy）と地面を特定する。
		// PlayerとEnemyで押し返しの計算は同一なので、DeathComponentの有無だけで分岐すればよい
		const auto isRider{ [](constant::Tag tag) noexcept
			{
			    return tag == constant::Tag::Player || tag == constant::Tag::Enemy;
			} };

		core::ecs::EntityId riderId{ core::ecs::INVALID_ENTITY_ID };
		core::ecs::EntityId groundId{ core::ecs::INVALID_ENTITY_ID };

		if (isRider(tagA.m_tag) && tagB.m_tag == constant::Tag::Ground)
		{
			riderId = a;
			groundId = b;
		}
		else if (tagA.m_tag == constant::Tag::Ground && isRider(tagB.m_tag))
		{
			riderId = b;
			groundId = a;
		}
		else
		{
			return;
		}

		auto& riderTransform = m_componentManager.get<component::movement::TransformComponent>(riderId);
		auto& groundTransform = m_componentManager.get<component::movement::TransformComponent>(groundId);
		auto& riderCollider = m_componentManager.get<component::combat::ColliderComponent>(riderId);
		auto& groundCollider = m_componentManager.get<component::combat::ColliderComponent>(groundId);
		auto& riderVelocity = m_componentManager.get<component::movement::VelocityComponent>(riderId);

		// 各コライダーの中心とAABBの境界を計算
		const core::Vector3 riderCenter{ riderTransform.m_position + riderCollider.m_offset };
		const core::Vector3 groundCenter{ groundTransform.m_position + groundCollider.m_offset };

		const float riderBottom{ riderCenter.y - riderCollider.m_size.y / 2.0f };
		const float groundTop{ groundCenter.y + groundCollider.m_size.y / 2.0f };

		// 地面より下、または地面に近い場合に補正して下端を地面の上端へ合わせる
		if (riderBottom > groundTop)
			return;

		riderTransform.m_position.y += groundTop - riderBottom;

		// 死亡中の敵は地面で反発してバウンドする（Safariの落下演出）。
		// 落下速度が閾値を下回ったら跳ねるのをやめて静止させ、着地済みとして記録する。
		// この着地フラグを見てEnemyDeathSystemがバウンド完了後に消失フェードを始める
		auto* death{ m_componentManager.tryGet<component::combat::DeathComponent>(riderId) };

		// 初回接地の時点で「地面に触れた」と記録する。EnemyDeathSystemはこれを見て
		// バウンド完了を待たずに落下死のガタガタ揺れを止める
		if (death != nullptr)
			death->m_hasTouchedGround = true;

		if (death != nullptr && riderVelocity.m_velocity.y < -DEATH_BOUNCE_MIN_SPEED)
		{
			riderVelocity.m_velocity.y = -riderVelocity.m_velocity.y * DEATH_BOUNCE_RESTITUTION;
		}
		else if (riderVelocity.m_velocity.y < 0.0f)
		{
			riderVelocity.m_velocity.y = 0.0f;
			if (death != nullptr)
				death->m_hasLanded = true;
		}
	}
} // namespace game::system::combat