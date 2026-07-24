#include "StageProp.h"
#include "game/component/movement/TransformComponent.h"
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
	    const core::Vector3& colliderSize)
	    : m_entity{ entityManager.create() }
	{
		component::movement::TransformComponent transform;
		transform.m_position = position;
		transform.m_rotation = rotation;
		transform.m_scale = scale;
		componentManager.add<component::movement::TransformComponent>(m_entity.getId(), transform);

		componentManager.add<component::visual::RenderComponent>(m_entity.getId(), { modelHandle });

		// コライダー指定（size全0）が無ければ判定を付けない（装飾のみの配置物）
		const bool hasCollider{ colliderSize.x != 0.0f || colliderSize.y != 0.0f || colliderSize.z != 0.0f };
		if (hasCollider)
		{
			component::combat::ColliderComponent collider;
			collider.m_size = colliderSize;
			// 中心を高さの半分に置く＝配置物の中心座標が箱の中心に一致する
			collider.m_offset = core::Vector3{ 0.0f, 0.0f, 0.0f };
			componentManager.add<component::combat::ColliderComponent>(m_entity.getId(), collider);
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
