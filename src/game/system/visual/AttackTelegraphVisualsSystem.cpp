#include "AttackTelegraphVisualsSystem.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/combat/ProjectileComponent.h"
#include "core/utility/Color.h"
#include <algorithm>

namespace
{
	// 予兆円を地面から僅かに浮かせる量（地面とのZファイティング回避）
	constexpr float GROUND_LIFT{ 2.0f };
} // namespace

namespace game::system::visual
{
	AttackTelegraphVisualsSystem::AttackTelegraphVisualsSystem(core::ecs::ComponentManager& componentManager,
	    core::iface::IRenderer& renderer)
	    : m_componentManager{ componentManager }
	    , m_renderer{ renderer }
	{
	}

	void AttackTelegraphVisualsSystem::update(float /*deltaTime*/)
	{
		// 描画のみ。予兆の状態はAttackComponentのワインドアップから読むため更新処理は不要
	}

	void AttackTelegraphVisualsSystem::draw()
	{
		using core::utility::Color;

		auto attackers{ m_componentManager.getAllEntities<component::combat::AttackComponent>() };
		for (auto attackerId : attackers)
		{
			const auto& attack{ m_componentManager.get<component::combat::AttackComponent>(attackerId) };

			// ワインドアップ中の近接攻撃だけを対象にする（弾は地面予兆を出さない）
			if (!attack.m_windupPending || attack.m_windupDelay <= 0.0f)
				continue;
			if (m_componentManager.has<component::combat::ProjectileComponent>(attackerId))
				continue;
			if (!m_componentManager.has<component::movement::TransformComponent>(attackerId))
				continue;

			const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(attackerId) };
			core::Vector3 center{ transform.m_position };
			center.y += GROUND_LIFT;

			const float radius{ attack.m_attackRange };
			// 溜めの進行度（0→1）。満ちきると着弾する
			const float progress{ std::clamp(1.0f - attack.m_windupTimer / attack.m_windupDelay, 0.0f, 1.0f) };

			// 危険範囲の下地 → 中心から満ちていく内側 → 外周リング、の順で重ねる
			m_renderer.drawGroundCircle(center, radius, Color::TELEGRAPH_BASE, true);
			m_renderer.drawGroundCircle(center, radius * progress, Color::TELEGRAPH_FILL, true);
			m_renderer.drawGroundCircle(center, radius, Color::TELEGRAPH_RING, false);
		}
	}
} // namespace game::system::visual
