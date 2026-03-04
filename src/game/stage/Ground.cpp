#include "Ground.h"
#include "game/component/TransformComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/ColliderComponent.h"

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

		componentManager.add<component::TransformComponent>(m_entity.getId(), transform);
		componentManager.add<component::RenderComponent>(m_entity.getId(), { modelHandle });

		component::ColliderComponent collider;
		collider.m_size = size;

		componentManager.add<component::ColliderComponent>(m_entity.getId(), collider);
	}

	core::ecs::EntityId Ground::getId() const
	{
		return m_entity.getId();
	}
}