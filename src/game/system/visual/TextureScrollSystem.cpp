#include "TextureScrollSystem.h"
#include "game/component/visual/RenderComponent.h"
#include <cmath>

namespace game::system::visual
{
	TextureScrollSystem::TextureScrollSystem(core::ecs::ComponentManager& componentManager)
	    : m_componentManager{ componentManager }
	{
	}

	void TextureScrollSystem::update(float deltaTime)
	{
		const auto entities{ m_componentManager.getAllEntities<component::visual::RenderComponent>() };

		for (const auto entityId : entities)
		{
			auto& render{ m_componentManager.get<component::visual::RenderComponent>(entityId) };
			if (render.m_scrollSpeedU == 0.0f && render.m_scrollSpeedV == 0.0f)
				continue;

			render.m_scrollOffsetU += render.m_scrollSpeedU * deltaTime;
			render.m_scrollOffsetV += render.m_scrollSpeedV * deltaTime;

			// テクスチャ1枚ぶんで見た目が一巡するので、値が際限なく増えないよう丸める。
			// 放置するとfloatの精度が落ちて動きがカクつく
			render.m_scrollOffsetU = std::fmod(render.m_scrollOffsetU, 1.0f);
			render.m_scrollOffsetV = std::fmod(render.m_scrollOffsetV, 1.0f);
		}
	}
} // namespace game::system::visual
