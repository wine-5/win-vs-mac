#include "Player.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/InputComponent.h"
#include "game/component/RenderComponent.h"

namespace game::actor
{
	Player::Player(core::ecs::EntityManager& entityManager,
		core::ecs::ComponentManager& componentManager,
		core::iface::IResourceManager& resourceManager)
		: m_entity(entityManager.create())
	{
		componentManager.add<component::TransformComponent>(m_entity.getId(), {});
		componentManager.add<component::VelocityComponent>(m_entity.getId(), {});
		componentManager.add<component::InputComponent>(m_entity.getId(), {});

		// モデルを読み込んでRenderComponentに設定する
		int handle = resourceManager.loadModel(PLAYER_MODEL_PATH);
		componentManager.add<component::RenderComponent>(m_entity.getId(), { handle });
	}

	core::ecs::EntityId Player::getId() const
	{
		return m_entity.getId();
	}
}