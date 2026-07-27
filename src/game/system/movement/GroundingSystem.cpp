#include "GroundingSystem.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/movement/GroundSurfaceComponent.h"
#include "game/component/movement/FallRecoveryComponent.h"
#include "game/component/combat/DeathComponent.h"
#include "game/component/combat/ProjectileComponent.h"
#include "game/component/TagComponent.h"
#include "game/constant/Tag.h"
#include "core/utility/Rotation.h"
#include <cmath>
#include <algorithm>

namespace
{
	// 面がこの高さまで上にあれば「今立っている面」とみなす（小さな段差を登れる猶予）
	constexpr float STEP_TOLERANCE{ 40.0f };
	// 天面がほぼ垂直な面は床として扱わない
	constexpr float MIN_UP_NORMAL{ 0.0001f };
	// 死亡中の敵が着地で反発する係数（CollisionSystemと揃える）
	constexpr float DEATH_BOUNCE_RESTITUTION{ 0.5f };
	// これより落下速度が遅くなったらバウンドをやめて静止させる
	constexpr float DEATH_BOUNCE_MIN_SPEED{ 20.0f };
	// これ未満の傾きは水平とみなして滑らせない
	constexpr float MIN_SLIDE_STEEPNESS{ 0.01f };
	// 滑り速度の上限（際限なく加速させない）
	constexpr float MAX_SLIDE_SPEED{ 1800.0f };
	// 滑らない足場に移ったとき、残った滑り速度が減衰する割合（毎秒）
	constexpr float SLIDE_DECAY_PER_SEC{ 6.0f };
	// 動く歩道に乗ったとき、運ぶ速度へ寄っていく割合（毎秒）。
	// 即座に合わせると乗った瞬間に弾かれたように見えるので少しだけ滑らかにする
	constexpr float CONVEYOR_BLEND_PER_SEC{ 8.0f };
} // namespace

namespace game::system::movement
{
	GroundingSystem::GroundingSystem(core::ecs::ComponentManager& componentManager)
	    : m_componentManager{ componentManager }
	{
	}

	void GroundingSystem::updateSlide(component::movement::VelocityComponent& velocity,
	    const core::Vector3& normal, float slideAccel, float deltaTime) const
	{
		// 法線の水平成分＝坂を下る向き。長さは傾きの強さ（水平面なら0）
		const core::Vector3 downhill{ normal.x, 0.0f, normal.z };
		const float steepness{ downhill.length() };

		if (slideAccel <= 0.0f || steepness < MIN_SLIDE_STEEPNESS)
		{
			// 滑らない足場では、残っている滑り速度を減衰させて止める
			const float decay{ 1.0f - std::min(SLIDE_DECAY_PER_SEC * deltaTime, 1.0f) };
			velocity.m_externalVelocity.x *= decay;
			velocity.m_externalVelocity.z *= decay;
			return;
		}

		const core::Vector3 direction{ downhill * (1.0f / steepness) };
		const float accel{ slideAccel * steepness * deltaTime };
		velocity.m_externalVelocity.x += direction.x * accel;
		velocity.m_externalVelocity.z += direction.z * accel;

		// 落ち続けて無限に速くならないよう頭打ちにする
		const float speed{ std::sqrt(velocity.m_externalVelocity.x * velocity.m_externalVelocity.x +
			                         velocity.m_externalVelocity.z * velocity.m_externalVelocity.z) };
		if (speed > MAX_SLIDE_SPEED)
		{
			const float scale{ MAX_SLIDE_SPEED / speed };
			velocity.m_externalVelocity.x *= scale;
			velocity.m_externalVelocity.z *= scale;
		}
	}

	void GroundingSystem::updateConveyor(component::movement::VelocityComponent& velocity,
	    const core::Vector3& conveyorVelocity, float deltaTime) const
	{
		const float blend{ std::min(CONVEYOR_BLEND_PER_SEC * deltaTime, 1.0f) };
		velocity.m_externalVelocity.x += (conveyorVelocity.x - velocity.m_externalVelocity.x) * blend;
		velocity.m_externalVelocity.z += (conveyorVelocity.z - velocity.m_externalVelocity.z) * blend;
	}

	core::Vector3 GroundingSystem::conveyorVelocityOf(core::ecs::EntityId surfaceId) const
	{
		const auto& surface{ m_componentManager.get<component::movement::GroundSurfaceComponent>(surfaceId) };
		if (surface.m_conveyorSpeed == 0.0f)
			return core::Vector3{};

		// 運ぶ向きは面のローカル+Z（配置物の長辺）をワールドへ回したもの。
		// 「坂を下る向き」ではなく面自身の向きなので、配置をY180度回すだけで逆走にできる。
		// テクスチャの流れる向きもローカル+Zに合わせてあるため、見た目と一致する
		const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(surfaceId) };
		const core::Vector3 forward{ core::utility::rotateEulerXYZ(
			core::Vector3{ 0.0f, 0.0f, 1.0f }, transform.m_rotation) };
		return forward * surface.m_conveyorSpeed;
	}

	bool GroundingSystem::surfaceHeightAt(core::ecs::EntityId surfaceId, float x, float z,
	    float& outHeight, core::Vector3& outNormal) const
	{
		const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(surfaceId) };
		const auto& surface{ m_componentManager.get<component::movement::GroundSurfaceComponent>(surfaceId) };

		const core::Vector3& center{ transform.m_position };
		const core::Vector3& rotation{ transform.m_rotation };
		const core::Vector3 halfSize{ surface.m_size * 0.5f };

		// 天面の法線と、天面上の一点（箱の中心から真上へ半分ずらした点）を回転で求める
		const core::Vector3 normal{ core::utility::rotateEulerXYZ(core::Vector3{ 0.0f, 1.0f, 0.0f }, rotation) };
		if (std::abs(normal.y) < MIN_UP_NORMAL)
			return false;

		const core::Vector3 top{ center + core::utility::rotateEulerXYZ(core::Vector3{ 0.0f, halfSize.y, 0.0f }, rotation) };

		// 平面 normal・(p - top) = 0 を y について解く
		const float height{ top.y - (normal.x * (x - top.x) + normal.z * (z - top.z)) / normal.y };

		// 求めた接地点を配置物のローカル座標へ戻し、箱の範囲内かを見る
		const core::Vector3 local{ core::utility::inverseRotateEulerXYZ(
			core::Vector3{ x - center.x, height - center.y, z - center.z }, rotation) };
		if (std::abs(local.x) > halfSize.x || std::abs(local.z) > halfSize.z)
			return false;

		outHeight = height;
		outNormal = normal;
		return true;
	}

	void GroundingSystem::update(float deltaTime)
	{
		const auto surfaces{ m_componentManager.getAllEntities<component::movement::GroundSurfaceComponent>() };
		if (surfaces.empty())
			return;

		const auto riders{ m_componentManager.getAllEntities<component::movement::VelocityComponent>() };

		for (const auto riderId : riders)
		{
			// 弾は接地させない
			if (m_componentManager.has<component::combat::ProjectileComponent>(riderId))
				continue;

			const auto* tag{ m_componentManager.tryGet<component::TagComponent>(riderId) };
			if (tag == nullptr ||
			    (tag->m_tag != constant::Tag::Player && tag->m_tag != constant::Tag::Enemy))
				continue;

			auto& transform{ m_componentManager.get<component::movement::TransformComponent>(riderId) };
			auto& velocity{ m_componentManager.get<component::movement::VelocityComponent>(riderId) };

			// モデル原点が足元なので、足の高さ＝positionのY
			const float foot{ transform.m_position.y };

			// 落下中はこのフレームで進んだぶんも探索範囲に入れる（速い落下ですり抜けるのを防ぐ）
			float reach{ STEP_TOLERANCE };
			if (velocity.m_velocity.y < 0.0f)
				reach += -velocity.m_velocity.y * deltaTime;

			// 足元にある面のうち最も高いものを選ぶ
			bool found{ false };
			float bestHeight{ 0.0f };
			core::Vector3 bestNormal{ 0.0f, 1.0f, 0.0f };
			float bestSlideAccel{ 0.0f };
			core::Vector3 bestConveyor{};
			for (const auto surfaceId : surfaces)
			{
				float height{ 0.0f };
				core::Vector3 normal{};
				if (!surfaceHeightAt(surfaceId, transform.m_position.x, transform.m_position.z, height, normal))
					continue;
				if (height > foot + reach)
					continue; // 頭上の面（別階層の床など）は無視する
				if (!found || height > bestHeight)
				{
					found = true;
					bestHeight = height;
					bestNormal = normal;
					bestSlideAccel = m_componentManager
					                     .get<component::movement::GroundSurfaceComponent>(surfaceId)
					                     .m_slideAccel;
					bestConveyor = conveyorVelocityOf(surfaceId);
				}
			}

			// 足元の床の高さを共有する。浮遊敵はこれを基準にホバー高度を決める
			velocity.m_hasGroundHeight = found;
			velocity.m_groundHeight = found ? bestHeight : 0.0f;

			// 接地している面に応じて外力を更新する（空中では減衰させる）
			const bool isStanding{ found && foot <= bestHeight + STEP_TOLERANCE };
			velocity.m_isGrounded = isStanding; // ジャンプの可否判定用にPhysicsSystemへ伝える

			// 動く歩道は運ぶ速度そのものが外力になるため、滑りとは併用せず排他にする。
			// 両方効かせると坂の下り勾配ぶんだけ速度が上乗せされ、データの値と挙動が合わなくなる
			const bool onConveyor{ isStanding && (bestConveyor.x != 0.0f || bestConveyor.z != 0.0f) };
			if (onConveyor)
				updateConveyor(velocity, bestConveyor, deltaTime);
			else
				updateSlide(velocity, isStanding ? bestNormal : core::Vector3{ 0.0f, 1.0f, 0.0f },
				    isStanding ? bestSlideAccel : 0.0f, deltaTime);

			// 面より下に沈んでいるときだけ持ち上げる。引き下げないので
			// 障害物（Box）の上に立っている状態を壊さない
			if (!found || foot >= bestHeight)
				continue;

			transform.m_position.y = bestHeight;

			// ここに立てたということは安全な足場。落下時の戻り先として覚えておく
			if (auto* recovery{ m_componentManager.tryGet<component::movement::FallRecoveryComponent>(riderId) })
			{
				recovery->m_lastSafePosition = transform.m_position;
				recovery->m_hasSafePosition = true;
			}

			// 着地処理（CollisionSystemの縦解決と同じ扱い）
			auto* death{ m_componentManager.tryGet<component::combat::DeathComponent>(riderId) };
			if (death != nullptr)
				death->m_hasTouchedGround = true;

			if (death != nullptr && velocity.m_velocity.y < -DEATH_BOUNCE_MIN_SPEED)
				velocity.m_velocity.y = -velocity.m_velocity.y * DEATH_BOUNCE_RESTITUTION;
			else if (velocity.m_velocity.y < 0.0f)
			{
				velocity.m_velocity.y = 0.0f;
				if (death != nullptr)
					death->m_hasLanded = true;
			}
		}
	}
} // namespace game::system::movement
