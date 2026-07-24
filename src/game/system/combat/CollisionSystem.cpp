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
		collectAabbs();

		// 押し返しが起きるのは 乗る側×地面側 だけ。全Entityの総当たりだと
		// 地面同士・敵同士といった何もしない組み合わせが大半を占めるため、そこを丸ごと省く
		for (auto& rider : m_riders)
		{
			for (const auto& ground : m_grounds)
			{
				if (isColliding(rider, ground))
					resolveCollision(rider, ground);
			}
		}
	}

	void CollisionSystem::collectAabbs()
	{
		m_riders.clear();
		m_grounds.clear();

		const auto entities{ m_componentManager.getAllEntities<component::combat::ColliderComponent>() };

		for (const auto id : entities)
		{
			const auto* tag{ m_componentManager.tryGet<component::TagComponent>(id) };
			if (tag == nullptr)
				continue;

			const bool isRider{ tag->m_tag == constant::Tag::Player || tag->m_tag == constant::Tag::Enemy };
			if (!isRider && tag->m_tag != constant::Tag::Ground)
				continue;

			const auto* transform{ m_componentManager.tryGet<component::movement::TransformComponent>(id) };
			if (transform == nullptr)
				continue;

			// 乗る側は速度を止める処理があるため、VelocityComponentが無いものは対象外
			if (isRider && !m_componentManager.has<component::movement::VelocityComponent>(id))
				continue;

			const auto& collider{ m_componentManager.get<component::combat::ColliderComponent>(id) };

			Aabb aabb{};
			aabb.m_id = id;
			aabb.m_center = transform->m_position + collider.m_offset;
			aabb.m_halfSize = collider.m_size * 0.5f;

			if (isRider)
				m_riders.push_back(aabb);
			else
				m_grounds.push_back(aabb);
		}
	}

	bool CollisionSystem::isColliding(const Aabb& a, const Aabb& b) noexcept
	{
		// 各軸の距離が「半サイズの和」以下なら、その軸は重なっている
		return std::abs(a.m_center.x - b.m_center.x) <= a.m_halfSize.x + b.m_halfSize.x &&
		       std::abs(a.m_center.y - b.m_center.y) <= a.m_halfSize.y + b.m_halfSize.y &&
		       std::abs(a.m_center.z - b.m_center.z) <= a.m_halfSize.z + b.m_halfSize.z;
	}

	void CollisionSystem::resolveCollision(Aabb& rider, const Aabb& ground)
	{
		// 軸ごとのめり込み量を計算する
		const core::Vector3 delta{ rider.m_center - ground.m_center };
		const float overlapX{ rider.m_halfSize.x + ground.m_halfSize.x - std::abs(delta.x) };
		const float overlapY{ rider.m_halfSize.y + ground.m_halfSize.y - std::abs(delta.y) };
		const float overlapZ{ rider.m_halfSize.z + ground.m_halfSize.z - std::abs(delta.z) };

		// isColliding を通っているので全軸で正のはずだが、安全のため負なら何もしない
		if (overlapX <= 0.0f || overlapY <= 0.0f || overlapZ <= 0.0f)
			return;

		// PlayerとEnemyで押し返しの計算は同一なので、DeathComponentの有無だけで分岐すればよい
		auto& riderTransform = m_componentManager.get<component::movement::TransformComponent>(rider.m_id);
		auto& riderVelocity = m_componentManager.get<component::movement::VelocityComponent>(rider.m_id);

		// 最小めり込み軸に沿って押し出す（Minimum Translation Vector）。
		// 床（縦に薄い）は上へ押し出して「乗る」、壁（横に薄い）は横へ押し出して「止まる」に
		// 自然と分岐する。押し出す向きは相手の中心から離れる方向。
		if (overlapY <= overlapX && overlapY <= overlapZ)
		{
			const float pushY{ (delta.y >= 0.0f) ? overlapY : -overlapY };
			resolveVertical(rider.m_id, riderTransform, riderVelocity, overlapY, delta.y);
			rider.m_center.y += pushY;
		}
		else if (overlapX <= overlapZ)
		{
			const float pushX{ (delta.x >= 0.0f) ? overlapX : -overlapX };
			riderTransform.m_position.x += pushX;
			rider.m_center.x += pushX;
			// 壁に向かう水平速度だけ止める（横滑りは残す）
			if ((delta.x >= 0.0f && riderVelocity.m_velocity.x < 0.0f) ||
			    (delta.x < 0.0f && riderVelocity.m_velocity.x > 0.0f))
				riderVelocity.m_velocity.x = 0.0f;
		}
		else
		{
			const float pushZ{ (delta.z >= 0.0f) ? overlapZ : -overlapZ };
			riderTransform.m_position.z += pushZ;
			rider.m_center.z += pushZ;
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