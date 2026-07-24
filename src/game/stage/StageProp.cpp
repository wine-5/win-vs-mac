#include "StageProp.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/GroundSurfaceComponent.h"
#include "game/component/visual/RenderComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/TagComponent.h"
#include "game/constant/Tag.h"

namespace game::stage
{
	StageProp::StageProp(core::ecs::EntityManager& entityManager,
	    core::ecs::ComponentManager& componentManager,
	    int modelHandle,
	    const core::Vector3& position,
	    const core::Vector3& rotation,
	    const core::Vector3& scale,
	    constant::PropCollision collision,
	    const core::Vector3& collisionSize)
	    : m_entity{ entityManager.create() }
	{
		component::movement::TransformComponent transform;
		transform.m_position = position;
		transform.m_rotation = rotation;
		transform.m_scale = scale;
		componentManager.add<component::movement::TransformComponent>(m_entity.getId(), transform);

		componentManager.add<component::visual::RenderComponent>(m_entity.getId(), { modelHandle });

		// 塞ぐ障害物はAABBで押し返し、歩ける面は傾きを考慮した接地計算に回す。
		// 床・坂にAABBを付けると「傾いた面」を表現できず、上に乗れず浮くため分けている
		if (collision == constant::PropCollision::Box)
		{
			component::combat::ColliderComponent collider;
			collider.m_size = collisionSize;
			// 配置物の中心座標がそのまま箱の中心になる
			collider.m_offset = core::Vector3{ 0.0f, 0.0f, 0.0f };
			componentManager.add<component::combat::ColliderComponent>(m_entity.getId(), collider);
		}
		else if (collision == constant::PropCollision::Ground)
		{
			component::movement::GroundSurfaceComponent surface{};
			surface.m_size = collisionSize;
			componentManager.add<component::movement::GroundSurfaceComponent>(m_entity.getId(), surface);
		}

		component::TagComponent tag{};
		tag.m_tag = constant::Tag::Ground;
		componentManager.add<component::TagComponent>(m_entity.getId(), tag);
	}

	core::ecs::EntityId StageProp::getId() const noexcept
	{
		return m_entity.getId();
	}
} // namespace game::stage
