#include "SafariEnemy.h"
#include "game/component/AIComponent.h"
#include "game/component/ai/RangeKeepAIComponent.h"

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

		// RangeKeepAISystemがこのコンポーネントの有無で距離維持型敵を判定する
		component::ai::RangeKeepAIComponent rangeKeep{};
		rangeKeep.m_preferredDistanceMin = PREFERRED_DISTANCE_MIN;
		rangeKeep.m_preferredDistanceMax = PREFERRED_DISTANCE_MAX;
		rangeKeep.m_hoverHeight = HOVER_HEIGHT;
		m_componentManager.add<component::ai::RangeKeepAIComponent>(m_entity.getId(), rangeKeep);
	}
} // namespace game::actor