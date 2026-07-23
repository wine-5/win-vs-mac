#include "TelegraphVisualsSystem.h"
#include "game/component/TelegraphComponent.h"
#include "core/utility/Color.h"

namespace
{
	// 予兆を地面から僅かに浮かせる量（地面とのZファイティング回避）
	constexpr float GROUND_LIFT{ 2.0f };
} // namespace

namespace game::system::visual
{
	TelegraphVisualsSystem::TelegraphVisualsSystem(core::ecs::ComponentManager& componentManager,
	    core::iface::IRenderer& renderer)
	    : m_componentManager{ componentManager }
	    , m_renderer{ renderer }
	{
	}

	void TelegraphVisualsSystem::update(float /*deltaTime*/)
	{
		// 描画のみ。予兆の状態はTelegraphComponentが持つため更新処理は不要
	}

	void TelegraphVisualsSystem::draw()
	{
		using core::utility::Color;

		auto entities{ m_componentManager.getAllEntities<component::TelegraphComponent>() };
		for (auto entityId : entities)
		{
			const auto& tel{ m_componentManager.get<component::TelegraphComponent>(entityId) };
			if (!tel.m_active || tel.m_radius <= 0.0f)
				continue;

			core::Vector3 center{ tel.m_center };
			center.y += GROUND_LIFT;

			// 危険範囲の下地 → 中心から満ちていく内側 → 外周リング、の順で重ねる
			if (tel.m_shape == component::TelegraphShape::Sector)
			{
				m_renderer.drawGroundSector(center, tel.m_facingRad, tel.m_radius, tel.m_halfAngleRad, Color::TELEGRAPH_BASE, true);
				m_renderer.drawGroundSector(center, tel.m_facingRad, tel.m_radius * tel.m_progress, tel.m_halfAngleRad, Color::TELEGRAPH_FILL, true);
				m_renderer.drawGroundSector(center, tel.m_facingRad, tel.m_radius, tel.m_halfAngleRad, Color::TELEGRAPH_RING, false);
			}
			else
			{
				m_renderer.drawGroundCircle(center, tel.m_radius, Color::TELEGRAPH_BASE, true);
				m_renderer.drawGroundCircle(center, tel.m_radius * tel.m_progress, Color::TELEGRAPH_FILL, true);
				m_renderer.drawGroundCircle(center, tel.m_radius, Color::TELEGRAPH_RING, false);
			}
		}
	}
} // namespace game::system::visual
