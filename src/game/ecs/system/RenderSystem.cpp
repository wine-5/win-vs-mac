#include "RenderSystem.h"
#include <DxLib.h>
#include "game/ecs/component/TransformComponent.h"
#include "game/ecs/component/RenderComponent.h"
#include "utility/LogUtil.h"

namespace game::ecs::system
{
	RenderSystem::RenderSystem(ComponentManager& componentManager, EntityId playerId)
		: m_componentManager(componentManager)
		, m_playerId(playerId)
	{
	}

	void RenderSystem::update(float deltaTime)
	{
		auto& transform = m_componentManager.get<component::TransformComponent>(m_playerId);
		auto& render = m_componentManager.get<component::RenderComponent>(m_playerId);

		if (!render.m_isVisible) return;

		if (render.m_modelHandle == -1)
		{
			utility::LogUtil::error("モデルが読み込まれていません");
			return;
		}

		MV1SetPosition(render.m_modelHandle, transform.m_position);
		MV1DrawModel(render.m_modelHandle);
	}
}