#include "SafariEnemy.h"
#include "game/component/AIComponent.h"
#include "game/component/ai/RangeKeepAIComponent.h"

namespace
{
	constexpr float DEG_TO_RAD{ 3.14159265358979323846f / 180.0f };
} // namespace

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
		rangeKeep.m_fireCooldown = m_enemyData.getFireCooldown();
		// JSONは度で持つのでラジアンへ変換して保持する
		rangeKeep.m_facingYawOffset = m_enemyData.getFacingYawOffset() * DEG_TO_RAD;
		m_componentManager.add<component::ai::RangeKeepAIComponent>(m_entity.getId(), rangeKeep);
	}
} // namespace game::actor