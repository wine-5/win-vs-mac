#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"

namespace game::actor
{
	constexpr const char* PLAYER_MODEL_PATH = "assets/Model/Player.mv1";

	/**
	 * @brief Playerのセットアップを担当するクラス
	 */
	class Player
	{
	public:
		Player(core::ecs::EntityManager& entityManager, core::ecs::ComponentManager& componentManager);
		core::ecs::EntityId getId() const;

	private:

		core::ecs::Entity m_entity;
	};
}