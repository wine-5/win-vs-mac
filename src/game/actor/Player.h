#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IResourceManager.h"
namespace game::actor
{
	constexpr const char* PLAYER_MODEL_PATH = "assets/model/Player.mv1";
	constexpr float PLAYER_MOVE_SPEED = 5.0f;

	/**
	 * @brief Playerのセットアップを担当するクラス
	 */
	class Player
	{
	public:
		Player(core::ecs::EntityManager& entityManager,
			core::ecs::ComponentManager& componentManager,
			core::iface::IResourceManager& resourceManager);
		core::ecs::EntityId getId() const;

	private:
		core::ecs::Entity m_entity;
	};
}