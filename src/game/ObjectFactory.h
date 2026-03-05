#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "game/actor/Player.h"
#include "core/interface/IResourceManager.h"
#include "game/data/PlayerData.h"
#include <memory>

namespace game
{
	/**
	 * @brief ゲームオブジェクトの生成・破棄を担当するクラス
	 */
	class ObjectFactory
	{
	public:
		ObjectFactory(core::ecs::EntityManager& entityManager, core::ecs::ComponentManager& componentManager,core::iface::IResourceManager& resourceManager);
		void init(int playerModelhandle, const data::PlayerData& playerData);

		actor::Player& getPlayer() const;

	private:
		core::ecs::EntityManager& m_entityManager;
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IResourceManager& m_resourceManager;

		std::unique_ptr<actor::Player> m_player; // init()で生成タイミングを遅らせるためunique_ptrで保持
	};
}