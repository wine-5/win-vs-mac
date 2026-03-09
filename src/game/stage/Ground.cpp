#include "Ground.h"
#include "game/component/TransformComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/ColliderComponent.h"
#include "game/constant/CollisionTag.h"
#include "game/data/GroundData.h"

namespace game::stage
{
	Ground::Ground(core::ecs::EntityManager& entityManager,
		core::ecs::ComponentManager& componentManager,
		int modelHandle,
		const game::data::GroundData& groundData)
		: m_entity(entityManager.create())
	{
		component::TransformComponent transform;
		transform.m_position = groundData.getPosition();
		transform.m_rotation = groundData.getRotation();
		transform.m_scale = groundData.getScale();

		componentManager.add<component::TransformComponent>(m_entity.getId(), transform);
		componentManager.add<component::RenderComponent>(m_entity.getId(), { modelHandle });

		component::ColliderComponent collider;
		collider.m_size = groundData.getColliderSize();
		collider.m_offset = groundData.getColliderOffset();
		collider.m_tag = constant::CollisionTag::Ground;

		componentManager.add<component::ColliderComponent>(m_entity.getId(), collider);
	}

	core::ecs::EntityId Ground::getId() const
	{
		return m_entity.getId();
	}
}