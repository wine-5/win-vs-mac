#include "Ground.h"
#include "game/component/TransformComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/ColliderComponent.h"
#include "game/constant/CollisionTag.h"

namespace game::stage
{
	Ground::Ground(core::ecs::EntityManager& entityManager,
		core::ecs::ComponentManager& componentManager,
		int modelHandle,
		core::Vector3 position,
		core::Vector3 size)
		: m_entity(entityManager.create())
	{
		component::TransformComponent transform;
		transform.m_position = position;
		transform.m_scale = core::Vector3(1.0f, 1.0f, 1.0f);

		componentManager.add<component::TransformComponent>(m_entity.getId(), transform);
		componentManager.add<component::RenderComponent>(m_entity.getId(), { modelHandle });

		component::ColliderComponent collider;
		collider.m_size = size;
		collider.m_tag = constant::CollisionTag::Ground;

		componentManager.add<component::ColliderComponent>(m_entity.getId(), collider);
	}

	core::ecs::EntityId Ground::getId() const
	{
		return m_entity.getId();
	}
}