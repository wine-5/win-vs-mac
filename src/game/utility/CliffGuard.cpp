#include "CliffGuard.h"
#include "GroundQuery.h"
#include "game/component/ai/CliffAvoidanceComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/VelocityComponent.h"

namespace game::utility
{
	bool canStepToward(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId entityId, const core::Vector3& direction)
	{
		const auto* avoidance{ componentManager.tryGet<component::ai::CliffAvoidanceComponent>(entityId) };
		if (avoidance == nullptr)
			return true;

		// 空中にいる間は判定しない。落下や吹き飛びの最中に足を止めても意味が無く、
		// 空中で固まって見えるだけになる
		const auto* velocity{ componentManager.tryGet<component::movement::VelocityComponent>(entityId) };
		if (velocity == nullptr || !velocity->m_isGrounded)
			return true;

		const auto& transform{ componentManager.get<component::movement::TransformComponent>(entityId) };
		const float foot{ transform.m_position.y };
		const float probeX{ transform.m_position.x + direction.x * avoidance->m_probeDistance };
		const float probeZ{ transform.m_position.z + direction.z * avoidance->m_probeDistance };

		const auto ground{ findGround(componentManager, probeX, probeZ, foot + avoidance->m_stepUpTolerance) };
		if (!ground.has_value())
			return false; // その先は奈落

		// 段差程度なら降りてよい。それより深ければ崖とみなす
		return foot - ground->m_height <= avoidance->m_maxStepDown;
	}

	bool hasFootingAt(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId entityId, float x, float z, float referenceHeight)
	{
		const auto* avoidance{ componentManager.tryGet<component::ai::CliffAvoidanceComponent>(entityId) };
		if (avoidance == nullptr)
			return true;

		return findGround(componentManager, x, z, referenceHeight + avoidance->m_stepUpTolerance).has_value();
	}
} // namespace game::utility
