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
		constexpr float preferredDistanceMin{ 400.0f };
		constexpr float preferredDistanceMax{ 600.0f };
		constexpr float hoverHeight{ 180.0f };

		auto& ai = m_componentManager.get<component::AIComponent>(m_entity.getId());
		ai.m_behavior = constant::AIBehavior::RangeKeepDistance;
		ai.m_preferredDistanceMin = preferredDistanceMin;
		ai.m_preferredDistanceMax = preferredDistanceMax;
		ai.m_hoverHeight = hoverHeight;
	}
} // namespace game::actor