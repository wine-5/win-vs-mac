#pragma once
#include "IScene.h"
#include "game/ecs/EntityManager.h"
#include "game/ecs/ComponentManager.h"
#include "game/ecs/SystemManager.h"
#include "game/actor/Player.h"

namespace game::scene
{
	/**
	 * @brief インゲームのシーンクラス
	 */
	class InGameScene : public IScene
	{
	public:
		InGameScene();
		void update(float deltaTime) override;

	private:
		ecs::EntityManager m_entityManager;
		ecs::ComponentManager m_componentManager;
		ecs::SystemManager m_systemManager;

		actor::Player m_player;
	};
}