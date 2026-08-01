#include "StageProp.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/GroundSurfaceComponent.h"
#include "game/component/visual/RenderComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/TagComponent.h"
#include "game/component/stage/DestructibleComponent.h"
#include "game/constant/Tag.h"

namespace game::stage
{
	StageProp::StageProp(core::ecs::EntityManager& entityManager,
	    core::ecs::ComponentManager& componentManager,
	    const StagePropParams& params)
	    : m_entity{ entityManager.create() }
	{
		component::movement::TransformComponent transform;
		transform.m_position = params.m_position;
		transform.m_rotation = params.m_rotation;
		transform.m_scale = params.m_scale;
		componentManager.add<component::movement::TransformComponent>(m_entity.getId(), transform);

		component::visual::RenderComponent render{};
		render.m_modelHandle = params.m_modelHandle;
		render.m_uvScaleU = params.m_uvScaleU;
		render.m_uvScaleV = params.m_uvScaleV;
		render.m_scrollSpeedU = params.m_scrollSpeedU;
		render.m_scrollSpeedV = params.m_scrollSpeedV;
		componentManager.add<component::visual::RenderComponent>(m_entity.getId(), render);

		// 塞ぐ障害物はAABBで押し返し、歩ける面は傾きを考慮した接地計算に回す。
		// 床・坂にAABBを付けると「傾いた面」を表現できず、上に乗れず浮くため分けている
		if (params.m_collision == constant::PropCollision::Box)
		{
			component::combat::ColliderComponent collider;
			collider.m_size = params.m_collisionSize;
			// 配置物の中心座標がそのまま箱の中心になる
			collider.m_offset = core::Vector3{ 0.0f, 0.0f, 0.0f };
			// 見た目と同じ向きに箱を傾ける（斜めに置いたブロックで実物とズレないように）
			collider.m_rotationY = params.m_rotation.y;
			componentManager.add<component::combat::ColliderComponent>(m_entity.getId(), collider);
		}
		else if (params.m_collision == constant::PropCollision::Ground)
		{
			component::movement::GroundSurfaceComponent surface{};
			surface.m_size = params.m_collisionSize;
			surface.m_slideAccel = params.m_slideAccel;
			surface.m_conveyorSpeed = params.m_conveyorSpeed;
			componentManager.add<component::movement::GroundSurfaceComponent>(m_entity.getId(), surface);
		}

		// 壊せる配置物は、床・壁と同じ立方体でも攻撃の対象になる点が違う。
		// タグで種別を、Componentで壊れ方を表す
		component::TagComponent tag{};
		tag.m_tag = params.m_hitsToBreak > 0 ? constant::Tag::Destructible : constant::Tag::Ground;
		componentManager.add<component::TagComponent>(m_entity.getId(), tag);

		if (params.m_hitsToBreak > 0)
		{
			component::stage::DestructibleComponent destructible{};
			destructible.m_hitsToBreak = params.m_hitsToBreak;
			destructible.m_intactHandle = params.m_modelHandle;
			destructible.m_fracturedHandle = params.m_fracturedHandle;
			destructible.m_crackTextures = params.m_crackTextures;
			componentManager.add<component::stage::DestructibleComponent>(m_entity.getId(), destructible);
		}
	}

	core::ecs::EntityId StageProp::getId() const noexcept
	{
		return m_entity.getId();
	}
} // namespace game::stage
