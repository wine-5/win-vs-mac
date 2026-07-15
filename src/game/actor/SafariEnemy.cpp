#include "SafariEnemy.h"
#include "game/component/AIComponent.h"

namespace game::actor
{
	void SafariEnemy::setupAnimation()
	{
		// Safariの敵はアニメーションを持たないため空実装
	}

	void SafariEnemy::setupAI()
	{
		// TODO: 個別のjsonを作るタイミングでjsonへ移植する
		constexpr float PREFERRED_DISTANCE_MIN{ 400.0f };
		constexpr float PREFERRED_DISTANCE_MAX{ 600.0f };
		constexpr float HOVER_HEIGHT{ 180.0f };

		auto& ai = m_componentManager.get<component::AIComponent>(m_entity.getId());
		ai.m_behavior = constant::AIBehavior::RangeKeepDistance;
		ai.m_preferredDistanceMin = PREFERRED_DISTANCE_MIN;
		ai.m_preferredDistanceMax = PREFERRED_DISTANCE_MAX;
		ai.m_hoverHeight = HOVER_HEIGHT;
	}
} // namespace game::actor