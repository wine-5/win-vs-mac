#include "CollisionSystem.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/TagComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/combat/DeathComponent.h"
#include "game/constant/Tag.h"
#include "core/utility/Rotation.h"
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
		collectBoxes();

		// 押し返しが起きるのは 乗る側×地面側 だけ。全Entityの総当たりだと
		// 地面同士・敵同士といった何もしない組み合わせが大半を占めるため、そこを丸ごと省く
		for (auto& rider : m_riders)
		{
			for (const auto& ground : m_grounds)
				resolveCollision(rider, ground);
		}
	}

	void CollisionSystem::collectBoxes()
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

			Box box{};
			box.m_id = id;
			box.m_center = transform->m_position + collider.m_offset;
			box.m_halfSize = collider.m_size * 0.5f;
			box.m_yaw = collider.m_rotationY;

			if (isRider)
				m_riders.push_back(box);
			else
				m_grounds.push_back(box);
		}
	}

	void CollisionSystem::toGroundLocal(const Box& rider, const Box& ground,
	    core::Vector3& outLocalDelta, core::Vector3& outLocalHalfSize) noexcept
	{
		const core::Vector3 yaw{ 0.0f, ground.m_yaw, 0.0f };
		outLocalDelta = core::utility::inverseRotateEulerXYZ(rider.m_center - ground.m_center, yaw);

		// 乗る側の箱を地面側の向きへ傾けると、軸並行では外接する箱まで広がる
		const float cosYaw{ std::abs(std::cos(ground.m_yaw)) };
		const float sinYaw{ std::abs(std::sin(ground.m_yaw)) };
		outLocalHalfSize = core::Vector3{
			rider.m_halfSize.x * cosYaw + rider.m_halfSize.z * sinYaw,
			rider.m_halfSize.y,
			rider.m_halfSize.x * sinYaw + rider.m_halfSize.z * cosYaw
		};
	}

	void CollisionSystem::resolveCollision(Box& rider, const Box& ground)
	{
		// 地面側の向きに合わせた座標系へ移すと、傾いた配置物でも軸並行の判定で済む
		core::Vector3 delta{};
		core::Vector3 riderHalfSize{};
		toGroundLocal(rider, ground, delta, riderHalfSize);

		// 軸ごとのめり込み量を計算する。1つでも0以下なら離れている
		const float overlapX{ riderHalfSize.x + ground.m_halfSize.x - std::abs(delta.x) };
		const float overlapY{ riderHalfSize.y + ground.m_halfSize.y - std::abs(delta.y) };
		const float overlapZ{ riderHalfSize.z + ground.m_halfSize.z - std::abs(delta.z) };
		if (overlapX <= 0.0f || overlapY <= 0.0f || overlapZ <= 0.0f)
			return;

		// PlayerとEnemyで押し返しの計算は同一なので、DeathComponentの有無だけで分岐すればよい
		auto& riderTransform = m_componentManager.get<component::movement::TransformComponent>(rider.m_id);
		auto& riderVelocity = m_componentManager.get<component::movement::VelocityComponent>(rider.m_id);

		// 最小めり込み軸に沿って押し出す（Minimum Translation Vector）。
		// 床（縦に薄い）は上へ押し出して「乗る」、壁（横に薄い）は横へ押し出して「止まる」に
		// 自然と分岐する。押し出す向きは相手の中心から離れる方向。
		// 縦はY軸まわりの回転で変わらないので、ローカルとワールドで同じ向きになる
		if (overlapY <= overlapX && overlapY <= overlapZ)
		{
			const float pushY{ (delta.y >= 0.0f) ? overlapY : -overlapY };
			resolveVertical(rider.m_id, riderTransform, riderVelocity, overlapY, delta.y);
			rider.m_center.y += pushY;
			return;
		}

		core::Vector3 localPush{};
		if (overlapX <= overlapZ)
			localPush.x = (delta.x >= 0.0f) ? overlapX : -overlapX;
		else
			localPush.z = (delta.z >= 0.0f) ? overlapZ : -overlapZ;

		// 押し出しをワールドへ戻す。傾いた壁では斜め方向のずらしになる
		const core::Vector3 push{ core::utility::rotateEulerXYZ(localPush, core::Vector3{ 0.0f, ground.m_yaw, 0.0f }) };
		riderTransform.m_position.x += push.x;
		riderTransform.m_position.z += push.z;
		rider.m_center.x += push.x;
		rider.m_center.z += push.z;

		// 壁へ向かう速度成分だけ打ち消す（壁沿いの横滑りは残す）
		const float pushLength{ std::sqrt(push.x * push.x + push.z * push.z) };
		if (pushLength <= 0.0f)
			return;

		const core::Vector3 normal{ push.x / pushLength, 0.0f, push.z / pushLength };
		const float into{ riderVelocity.m_velocity.x * normal.x + riderVelocity.m_velocity.z * normal.z };
		if (into >= 0.0f)
			return;

		riderVelocity.m_velocity.x -= normal.x * into;
		riderVelocity.m_velocity.z -= normal.z * into;
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