#pragma once
#include "game/ecs/ISystem.h"
#include "game/ecs/ComponentManager.h"
#include "game/ecs/Entity.h"

namespace game::ecs::system
{
	/**
	* @brief 3Dモデルを描画するSystem
	*/
	class RenderSystem : public ISystem
	{
	public:
		RenderSystem(ComponentManager& componentManager, EntityId playerId);
		void update(float deltaTime) override;

	private:
		ComponentManager& m_componentManager;
		EntityId m_playerId;
	};
}