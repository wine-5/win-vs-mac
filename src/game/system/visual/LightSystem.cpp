#include "LightSystem.h"
#include "core/interface/ILighting.h"
#include "game/component/visual/LightComponent.h"
#include "game/component/movement/TransformComponent.h"
#include <algorithm>

namespace game::system::visual
{
	LightSystem::LightSystem(core::ecs::ComponentManager& componentManager, core::iface::ILighting& lighting)
	    : m_componentManager{ componentManager }
	    , m_lighting{ lighting }
	{
	}

	LightSystem::~LightSystem()
	{
		for (const auto& [entityId, handle] : m_lightsByEntity)
			m_lighting.destroyPointLight(handle);
	}

	void LightSystem::update(float deltaTime)
	{
		const auto entities{ m_componentManager.getAllEntities<component::visual::LightComponent>() };

		for (const auto entityId : entities)
		{
			auto& light{ m_componentManager.get<component::visual::LightComponent>(entityId) };

			const auto* transform{ m_componentManager.tryGet<component::movement::TransformComponent>(entityId) };
			if (transform == nullptr)
				continue;

			const core::Vector3 position{ transform->m_position + light.m_offset };

			// 初回だけ光源を作り、以降は位置だけ追従させる
			if (light.m_lightHandle == -1)
			{
				light.m_lightHandle = m_lighting.createPointLight(
				    position, light.m_range, light.m_r, light.m_g, light.m_b);
				if (light.m_lightHandle != -1)
					m_lightsByEntity[entityId] = light.m_lightHandle;
				continue;
			}

			m_lighting.setPointLightPosition(light.m_lightHandle, position);
		}

		destroyLightsOfLostEntities(entities);
	}

	void LightSystem::destroyLightsOfLostEntities(const std::vector<core::ecs::EntityId>& aliveEntities)
	{
		if (m_lightsByEntity.empty())
			return;

		// 今フレームに存在しなかったEntityの光源を消す。
		// 敵が倒されるとEntityごと消えるため、これをしないと死んだ場所に光が残る
		for (auto it{ m_lightsByEntity.begin() }; it != m_lightsByEntity.end();)
		{
			const bool isAlive{ std::find(aliveEntities.begin(), aliveEntities.end(), it->first) != aliveEntities.end() };
			if (isAlive)
			{
				++it;
				continue;
			}

			m_lighting.destroyPointLight(it->second);
			it = m_lightsByEntity.erase(it);
		}
	}
} // namespace game::system::visual
