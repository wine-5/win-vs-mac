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
		auto& ai = m_componentManager.get<component::AIComponent>(m_entity.getId());
		ai.m_behavior = constant::AIBehavior::RangeKeepDistance;

		// RangeKeepAISystemがこのコンポーネントの有無で距離維持型敵を判定する。
		// 距離維持・浮遊高度のパラメータはsafariData.jsonのgameplayから読み込む
		component::ai::RangeKeepAIComponent rangeKeep{};
		rangeKeep.m_preferredDistanceMin = m_enemyData.getPreferredDistanceMin();
		rangeKeep.m_preferredDistanceMax = m_enemyData.getPreferredDistanceMax();
		rangeKeep.m_hoverHeight = m_enemyData.getHoverHeight();
		m_componentManager.add<component::ai::RangeKeepAIComponent>(m_entity.getId(), rangeKeep);
	}
} // namespace game::actor