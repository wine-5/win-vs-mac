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

		// 各コライダーの中心と、軸ごとのめり込み量を計算する
		const core::Vector3 riderCenter{ riderTransform.m_position + riderCollider.m_offset };
		const core::Vector3 groundCenter{ groundTransform.m_position + groundCollider.m_offset };

		const core::Vector3 delta{ riderCenter - groundCenter };
		const float overlapX{ (riderCollider.m_size.x + groundCollider.m_size.x) / 2.0f - std::abs(delta.x) };
		const float overlapY{ (riderCollider.m_size.y + groundCollider.m_size.y) / 2.0f - std::abs(delta.y) };
		const float overlapZ{ (riderCollider.m_size.z + groundCollider.m_size.z) / 2.0f - std::abs(delta.z) };

		// isColliding を通っているので全軸で正のはずだが、安全のため負なら何もしない
		if (overlapX <= 0.0f || overlapY <= 0.0f || overlapZ <= 0.0f)
			return;

		// 最小めり込み軸に沿って押し出す（Minimum Translation Vector）。
		// 床（縦に薄い）は上へ押し出して「乗る」、壁（横に薄い）は横へ押し出して「止まる」に
		// 自然と分岐する。押し出す向きは相手の中心から離れる方向。
		if (overlapY <= overlapX && overlapY <= overlapZ)
			resolveVertical(riderId, riderTransform, riderVelocity, overlapY, delta.y);
		else if (overlapX <= overlapZ)
		{
			riderTransform.m_position.x += (delta.x >= 0.0f) ? overlapX : -overlapX;
			// 壁に向かう水平速度だけ止める（横滑りは残す）
			if ((delta.x >= 0.0f && riderVelocity.m_velocity.x < 0.0f) ||
			    (delta.x < 0.0f && riderVelocity.m_velocity.x > 0.0f))
				riderVelocity.m_velocity.x = 0.0f;
		}
		else
		{
			riderTransform.m_position.z += (delta.z >= 0.0f) ? overlapZ : -overlapZ;
			if ((delta.z >= 0.0f && riderVelocity.m_velocity.z < 0.0f) ||
			    (delta.z < 0.0f && riderVelocity.m_velocity.z > 0.0f))
				riderVelocity.m_velocity.z = 0.0f;
		}
	}

	void CollisionSystem::resolveVertical(core::ecs::EntityId riderId,
	    component::movement::TransformComponent& riderTransform,
	    component::movement::VelocityComponent& riderVelocity,
	    float overlapY, float deltaY)
	{
		if (deltaY >= 0.0f)
		{
			// riderが上＝地面に乗る。上端を相手の上端へ合わせる
			riderTransform.m_position.y += overlapY;

			// 死亡中の敵は地面で反発してバウンドする（Safariの落下演出）。
			// 落下速度が閾値を下回ったら跳ねるのをやめて静止させ、着地済みとして記録する。
			// この着地フラグを見てEnemyDeathSystemがバウンド完了後に消失フェードを始める
			auto* death{ m_componentManager.tryGet<component::combat::DeathComponent>(riderId) };

			// 初回接地の時点で「地面に触れた」と記録する。EnemyDeathSystemはこれを見て
			// バウンド完了を待たずに落下死のガタガタ揺れを止める
			if (death != nullptr)
				death->m_hasTouchedGround = true;

			if (death != nullptr && riderVelocity.m_velocity.y < -DEATH_BOUNCE_MIN_SPEED)
				riderVelocity.m_velocity.y = -riderVelocity.m_velocity.y * DEATH_BOUNCE_RESTITUTION;
			else if (riderVelocity.m_velocity.y < 0.0f)
			{
				riderVelocity.m_velocity.y = 0.0f;
				if (death != nullptr)
					death->m_hasLanded = true;
			}
		}
		else
		{
			// riderが下＝天井に頭をぶつけた。下へ押し戻し、上向き速度を止める
			riderTransform.m_position.y -= overlapY;
			if (riderVelocity.m_velocity.y > 0.0f)
				riderVelocity.m_velocity.y = 0.0f;
		}
	}
} // namespace game::system::combat