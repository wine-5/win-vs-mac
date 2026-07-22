#include "SafariEnemy.h"
#include "game/component/AIComponent.h"
#include "game/component/ai/RangeKeepAIComponent.h"
#include "game/component/ai/PatrolComponent.h"
#include <numbers>
#include <random>

namespace
{
	constexpr float DEG_TO_RAD{ std::numbers::pi_v<float> / 180.0f };

	/**
	 * @brief ストレイフ（周回）の向きを個体ごとにランダムに決める
	 * @return 右回り(+1) または 左回り(-1)
	 */
	int pickStrafeDirection()
	{
		static std::mt19937 rng{ std::random_device{}() };
		static std::uniform_int_distribution<int> dist{ 0, 1 };
		return dist(rng) == 0 ? 1 : -1;
	}
} // namespace

namespace game::actor
{
	void SafariEnemy::setupAnimation()
	{
		// Safariの敵はアニメーションを持たないため空実装
	}

	void SafariEnemy::setupAI()
	{
		// RangeKeepAISystemがこのコンポーネントの有無で距離維持型敵を判定する。
		// 距離維持・浮遊高度のパラメータはsafariData.jsonのgameplayから読み込む
		component::ai::RangeKeepAIComponent rangeKeep{};
		rangeKeep.m_preferredDistanceMin = m_enemyData.getPreferredDistanceMin();
		rangeKeep.m_preferredDistanceMax = m_enemyData.getPreferredDistanceMax();
		rangeKeep.m_hoverHeight = m_enemyData.getHoverHeight();
		// 周回の向きを個体ごとにランダムに固定する（複数体が散開して包囲する隊形になる）
		rangeKeep.m_strafeDirection = pickStrafeDirection();
		rangeKeep.m_fireCooldown = m_enemyData.getFireCooldown();
		// JSONは度で持つのでラジアンへ変換して保持する
		rangeKeep.m_facingYawOffset = m_enemyData.getFacingYawOffset() * DEG_TO_RAD;
		m_componentManager.add<component::ai::RangeKeepAIComponent>(m_entity.getId(), rangeKeep);
		// 索敵範囲外の徘徊状態は共通のPatrolComponentが持つ
		m_componentManager.add<component::ai::PatrolComponent>(m_entity.getId(), {});
	}
} // namespace game::actor