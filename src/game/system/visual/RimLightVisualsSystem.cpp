#include "RimLightVisualsSystem.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/visual/RenderComponent.h"
#include "game/component/visual/RimLightComponent.h"

namespace game::system::visual
{
	RimLightVisualsSystem::RimLightVisualsSystem(core::ecs::ComponentManager& componentManager,
	    core::iface::IRenderer& renderer)
	    : m_componentManager{ componentManager }
	    , m_renderer{ renderer }
	{
	}

	void RimLightVisualsSystem::update(float /*deltaTime*/)
	{
		// 描画のみ。輪郭の太さ・色は RimLightComponent から読むため更新処理は不要
	}

	void RimLightVisualsSystem::draw()
	{
		for (const auto entityId :
		    m_componentManager.getAllEntities<component::visual::RimLightComponent>())
		{
			const auto* render{ m_componentManager.tryGet<component::visual::RenderComponent>(entityId) };
			if (render == nullptr || !render->m_isVisible || render->m_modelHandle == -1)
				continue;

			const auto* transform{
				m_componentManager.tryGet<component::movement::TransformComponent>(entityId)
			};
			if (transform == nullptr)
				continue;

			const auto& rim{ m_componentManager.get<component::visual::RimLightComponent>(entityId) };
			m_renderer.drawModelOutline(render->m_modelHandle, transform->m_position,
			    transform->m_rotation, transform->m_scale, rim.m_thickness, rim.m_color);
		}
	}
} // namespace game::system::visual
