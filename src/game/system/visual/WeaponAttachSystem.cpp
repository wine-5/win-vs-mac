#include "WeaponAttachSystem.h"
#include "game/component/visual/WeaponAttachComponent.h"
#include "game/component/visual/RenderComponent.h"
#include "core/interface/ILogger.h"
#include "core/utility/Log.h"

namespace game::system::visual
{
	WeaponAttachSystem::WeaponAttachSystem(core::ecs::ComponentManager& componentManager,
	    core::iface::IRenderer& renderer)
	    : m_componentManager{ componentManager }
	    , m_renderer{ renderer }
	{
	}

	void WeaponAttachSystem::update(float deltaTime)
	{
		auto entities{ m_componentManager.getAllEntities<component::visual::WeaponAttachComponent>() };
		for (auto entityId : entities)
		{
			auto& attach{ m_componentManager.get<component::visual::WeaponAttachComponent>(entityId) };
			if (attach.m_isResolved)
				continue;

			if (!m_componentManager.has<component::visual::RenderComponent>(entityId))
				continue;

			// モデルの読み込みが済むまでは検索できないため、次のフレームへ持ち越す
			const auto& render{ m_componentManager.get<component::visual::RenderComponent>(entityId) };
			if (render.m_modelHandle == -1)
				continue;

			attach.m_isResolved = true;
			attach.m_frameIndex = m_renderer.findModelFrame(render.m_modelHandle, attach.m_frameName);
			if (attach.m_frameIndex >= 0)
				continue;

			// 見つからなかったときだけ候補を出す。正しいボーン名はここから拾って
			// 装着側の指定を直す
			core::log::error("[WeaponAttach] entity={} 装着先ボーン '{}' が見つかりません",
			    entityId, attach.m_frameName);
			const auto frameNames{ m_renderer.getModelFrameNames(render.m_modelHandle) };
			for (size_t i{ 0 }; i < frameNames.size(); ++i)
				core::log::error("[WeaponAttach]   [{}] {}", i, frameNames[i]);
		}
	}
} // namespace game::system::visual
