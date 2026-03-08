#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/Vector3.h"

namespace game::actor
{
	/**
	 * @brief Playerのセットアップを担当するクラス
	 */
	class Player
	{
	public:
		Player(core::ecs::EntityManager& entityManager,
			core::ecs::ComponentManager& componentManager,
			int modelHandle,
			core::Vector3 colliderSize,
			core::Vector3 colliderOffset);

		core::ecs::EntityId getId() const;

	private:
		core::ecs::Entity m_entity;
	};
}