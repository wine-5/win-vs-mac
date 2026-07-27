#include "GroundQuery.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/GroundSurfaceComponent.h"
#include "core/utility/Rotation.h"
#include <cmath>

namespace
{
	// 天面がほぼ垂直な面は床として扱わない
	constexpr float MIN_UP_NORMAL{ 0.0001f };
} // namespace

namespace game::utility
{
	bool surfaceTopAt(core::ecs::ComponentManager& componentManager, core::ecs::EntityId surfaceId,
	    float x, float z, float& outHeight, core::Vector3& outNormal)
	{
		const auto& transform{ componentManager.get<component::movement::TransformComponent>(surfaceId) };
		const auto& surface{ componentManager.get<component::movement::GroundSurfaceComponent>(surfaceId) };

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

	std::optional<GroundHit> findGround(core::ecs::ComponentManager& componentManager,
	    float x, float z, float ceiling)
	{
		const auto surfaces{ componentManager.getAllEntities<component::movement::GroundSurfaceComponent>() };

		std::optional<GroundHit> best{};
		for (const auto surfaceId : surfaces)
		{
			float height{ 0.0f };
			core::Vector3 normal{};
			if (!surfaceTopAt(componentManager, surfaceId, x, z, height, normal))
				continue;
			if (height > ceiling)
				continue; // 頭上の面（別階層の床など）は無視する
			if (!best.has_value() || height > best->m_height)
				best = GroundHit{ height, normal, surfaceId };
		}
		return best;
	}
} // namespace game::utility
