#include "ProjectileBlockSystem.h"
#include "game/component/combat/ProjectileComponent.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/TagComponent.h"
#include "game/constant/Tag.h"
#include "core/utility/Rotation.h"
#include <algorithm>
#include <cmath>

namespace
{
	// 進行方向の成分がこれ未満の軸は「その軸には進んでいない」とみなす（0除算を避ける）
	constexpr float MIN_AXIS_MOVE{ 0.0001f };
} // namespace

namespace game::system::combat
{
	ProjectileBlockSystem::ProjectileBlockSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityManager& entityManager)
	    : m_componentManager{ componentManager }
	    , m_entityManager{ entityManager }
	{
	}

	void ProjectileBlockSystem::collectBlockers()
	{
		m_blockers.clear();

		const auto entities{ m_componentManager.getAllEntities<component::combat::ColliderComponent>() };
		for (const auto id : entities)
		{
			// 遮るのは配置物だけ。プレイヤー・敵のコライダーはここでは扱わない
			const auto* tag{ m_componentManager.tryGet<component::TagComponent>(id) };
			if (tag == nullptr || !constant::isStageProp(tag->m_tag))
				continue;

			const auto* transform{ m_componentManager.tryGet<component::movement::TransformComponent>(id) };
			if (transform == nullptr)
				continue;

			const auto& collider{ m_componentManager.get<component::combat::ColliderComponent>(id) };

			Box box{};
			box.m_center = transform->m_position + collider.m_offset;
			box.m_halfSize = collider.m_size * 0.5f;
			box.m_yaw = collider.m_rotationY;
			m_blockers.push_back(box);
		}
	}

	bool ProjectileBlockSystem::intersects(const Box& box, const core::Vector3& from, const core::Vector3& to,
	    float radius) noexcept
	{
		// 箱の向きに合わせた座標系へ移すと、傾いた壁でも軸並行の判定で済む。
		// 弾の半径ぶん箱を膨らませることで、弾を点として扱える
		const core::Vector3 yaw{ 0.0f, box.m_yaw, 0.0f };
		const core::Vector3 start{ core::utility::inverseRotateEulerXYZ(from - box.m_center, yaw) };
		const core::Vector3 end{ core::utility::inverseRotateEulerXYZ(to - box.m_center, yaw) };
		const core::Vector3 half{ box.m_halfSize.x + radius, box.m_halfSize.y + radius, box.m_halfSize.z + radius };

		const core::Vector3 move{ end - start };
		const float startAxis[3]{ start.x, start.y, start.z };
		const float moveAxis[3]{ move.x, move.y, move.z };
		const float halfAxis[3]{ half.x, half.y, half.z };

		// 各軸のスラブと線分が重なる区間を絞り込む（残れば交差している）
		float enter{ 0.0f };
		float exit{ 1.0f };
		for (int axis{ 0 }; axis < 3; ++axis)
		{
			if (std::abs(moveAxis[axis]) < MIN_AXIS_MOVE)
			{
				// その軸に進んでいないので、最初から範囲内でなければ当たらない
				if (std::abs(startAxis[axis]) > halfAxis[axis])
					return false;
				continue;
			}

			// near/far はWindowsヘッダのマクロと衝突するため別名にしている
			float axisEnter{ (-halfAxis[axis] - startAxis[axis]) / moveAxis[axis] };
			float axisExit{ (halfAxis[axis] - startAxis[axis]) / moveAxis[axis] };
			if (axisEnter > axisExit)
				std::swap(axisEnter, axisExit);

			enter = std::max(enter, axisEnter);
			exit = std::min(exit, axisExit);
			if (enter > exit)
				return false;
		}

		return true;
	}

	void ProjectileBlockSystem::update(float deltaTime)
	{
		const auto projectiles{ m_componentManager.getAllEntities<component::combat::ProjectileComponent>() };
		if (projectiles.empty())
			return;

		collectBlockers();
		if (m_blockers.empty())
			return;

		for (const auto id : projectiles)
		{
			if (m_componentManager.get<component::combat::ProjectileComponent>(id).m_penetratesWalls)
				continue;

			const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(id) };
			const auto& velocity{ m_componentManager.get<component::movement::VelocityComponent>(id) };

			// 弾は1フレームで自分の大きさより長く進むため、点で見るとすり抜ける。
			// このフレームで進んだぶんを線分にして、通り抜けた壁も拾う
			const core::Vector3 moved{ (velocity.m_velocity + velocity.m_externalVelocity) * deltaTime };
			const core::Vector3 previous{ transform.m_position - moved };

			// 当たり判定半径をそのまま弾の太さとして使う（見た目の大きさと揃う）
			const auto* attack{ m_componentManager.tryGet<component::combat::AttackComponent>(id) };
			const float radius{ attack != nullptr ? attack->m_attackRange : 0.0f };

			const bool blocked{ std::ranges::any_of(m_blockers,
				[&](const Box& box)
				{ return intersects(box, previous, transform.m_position, radius); }) };
			if (!blocked)
				continue;

			m_componentManager.removeAll(id);
			m_entityManager.destroy(core::ecs::Entity(id));
		}
	}
} // namespace game::system::combat
