#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/Vector3.h"

namespace game::stage
{
	/**
	 * @brief 地面のオブジェクト
	 */
	class Ground
	{
	public:
		Ground(core::ecs::EntityManager& entityManager,
			core::ecs::ComponentManager& compnentManager,
			int modelHandle,
			core::Vector3 position,
			core::Vector3 size);

		core::ecs::EntityId getId() const;

	private:
		core::ecs::Entity m_entity;
	};
}