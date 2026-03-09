#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/Vector3.h"

// 前方宣言
namespace game::data { class GroundData; }

namespace game::stage
{
	/**
	 * @brief 地面のオブジェクト
	 */
	class Ground
	{
	public:
		Ground(core::ecs::EntityManager& entityManager,
			core::ecs::ComponentManager& componentManager,
			int modelHandle,
			const game::data::GroundData& groundData);

		core::ecs::EntityId getId() const;

	private:
		core::ecs::Entity m_entity;
	};
}