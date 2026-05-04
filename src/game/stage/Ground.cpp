#include "Ground.h"
#include "game/component/TransformComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/ColliderComponent.h"
#include "game/component/TagComponent.h"
#include "game/constant/Tag.h"
#include "game/data/GroundData.h"

namespace game::stage
{
	Ground::Ground(core::ecs::EntityManager& entityManager,
		core::ecs::ComponentManager& componentManager,
		int modelHandle,
		const game::data::GroundData& groundData)
		: m_entity{entityManager.create()}
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
		componentManager.add<component::ColliderComponent>(m_entity.getId(), collider);

		component::TagComponent tag{};
		tag.m_tag = constant::Tag::Ground;
		componentManager.add<component::TagComponent>(m_entity.getId(), tag);
	}

	core::ecs::EntityId Ground::getId() const noexcept
	{
		return m_entity.getId();
	}
}