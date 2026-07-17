#include "PlayerChargeVisualsSystem.h"
#include "game/component/PlayerChargeComponent.h"
#include "core/utility/Color.h"
#include <cmath>
#include <numbers>

namespace
{
	constexpr float OUTER_RADIUS_MARGIN{ 1.05f };     // 画面対角の半分に対する余裕（線の根本を画面外に出す）
	constexpr float INNER_RADIUS_BASE{ 0.55f };       // 溜め開始時の中央の空き（画面高さ比）
	constexpr float INNER_RADIUS_CLOSE{ 0.25f };      // 最大溜めで中央へ迫る量（画面高さ比）
	constexpr float FLICKER_FREQUENCY{ 10.0f };       // 集中線のちらつき回数（回/秒）
	constexpr int LINE_COUNT_BASE{ 6 };               // 溜め開始時の線の本数
	constexpr int LINE_COUNT_GROWTH{ 18 };            // 最大溜めで増える本数
	constexpr float WEDGE_HALF_WIDTH_MIN{ 0.002f };   // くさびの根本半幅の最小（画面高さ比）
	constexpr float WEDGE_HALF_WIDTH_RAND{ 0.006f };  // くさびの根本半幅の乱数幅（画面高さ比）
	constexpr float WEDGE_WIDTH_GROWTH_BASE{ 0.35f }; // 溜め開始時の太さ倍率（ここから1.0へ育つ）

	/**
	 * @brief 整数の種から0.0〜1.0の疑似乱数を返す（毎フレーム同じ種なら同じ値になる決定的な乱数）
	 */
	float rand01(int seed)
	{
		const float value{ std::sin(static_cast<float>(seed) * 12.9898f) * 43758.5453f };
		return value - std::floor(value);
	}
} // namespace

namespace game::system
{
	PlayerChargeVisualsSystem::PlayerChargeVisualsSystem(core::ecs::ComponentManager& componentManager,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::ecs::EntityId playerId)
	    : m_componentManager{ componentManager }
	    , m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_playerId{ playerId }
	{
	}

	void PlayerChargeVisualsSystem::update(float deltaTime)
	{
		if (!m_componentManager.has<component::PlayerChargeComponent>(m_playerId))
			return;

		const auto& charge{ m_componentManager.get<component::PlayerChargeComponent>(m_playerId) };

		// 溜め中だけ演出時間を進め、離したらリセットする（次の溜めは同じ見た目から始まる）
		if (charge.m_isCharging)
			m_animationTime += deltaTime;
		else
			m_animationTime = 0.0f;
	}

	void PlayerChargeVisualsSystem::draw()
	{
		if (!m_componentManager.has<component::PlayerChargeComponent>(m_playerId))
			return;

		const auto& charge{ m_componentManager.get<component::PlayerChargeComponent>(m_playerId) };
		if (!charge.m_isCharging || charge.m_chargeRate <= 0.0f)
			return;

		const float screenW{ static_cast<float>(m_screen.getWidth()) };
		const float screenH{ static_cast<float>(m_screen.getHeight()) };
		const float centerX{ screenW * 0.5f };
		const float centerY{ screenH * 0.5f };

		// 線の根本は画面の外（対角の半分より少し外）から生やし、必ず画面端まで届かせる
		const float outerRadius{ std::sqrt(screenW * screenW + screenH * screenH) * 0.5f * OUTER_RADIUS_MARGIN };

		// 中央は空けて視界を確保する。溜めるほど中心へ迫り、画面が締まっていくように見せる
		const float innerRadius{ screenH * (INNER_RADIUS_BASE - INNER_RADIUS_CLOSE * charge.m_chargeRate) };

		// 溜めるほど線が増えて密度が上がる
		const int lineCount{ LINE_COUNT_BASE + static_cast<int>(charge.m_chargeRate * LINE_COUNT_GROWTH) };

		const unsigned int lineColor{ core::utility::Color::rgb(255, 200, 100) }; // オレンジ系

		// 一定間隔で乱数の種を切り替え、手描きの集中線が揺れているようなちらつきを出す
		const int flickerStep{ static_cast<int>(m_animationTime * FLICKER_FREQUENCY) };

		constexpr float PI{ static_cast<float>(std::numbers::pi) };
		const float angleStep{ 2.0f * PI / lineCount };

		// 溜めるほど太くなる（開始時は35%の細さから最大溜めで100%へ）
		const float widthGrowth{ WEDGE_WIDTH_GROWTH_BASE + (1.0f - WEDGE_WIDTH_GROWTH_BASE) * charge.m_chargeRate };

		for (int i = 0; i < lineCount; ++i)
		{
			const int seed{ i + flickerStep * 7919 }; // 線ごと・ちらつきごとに種を変える

			// 等間隔を基準に、角度・先端位置・太さを乱して手描きらしくする
			const float angle{ angleStep * i + (rand01(seed) - 0.5f) * angleStep * 0.8f };
			const float apexRadius{ innerRadius * (1.0f + (rand01(seed + 1) - 0.5f) * 0.3f) };
			const float halfWidth{ screenH * (WEDGE_HALF_WIDTH_MIN + rand01(seed + 2) * WEDGE_HALF_WIDTH_RAND) * widthGrowth };

			const float dirX{ std::cos(angle) };
			const float dirY{ std::sin(angle) };
			const float perpX{ -dirY };
			const float perpY{ dirX };

			// 画面外の根本（幅あり）から中心方向の先端（幅ゼロ）へ向かう、先細りのくさび形
			const int apexX{ static_cast<int>(centerX + dirX * apexRadius) };
			const int apexY{ static_cast<int>(centerY + dirY * apexRadius) };
			const int base1X{ static_cast<int>(centerX + dirX * outerRadius + perpX * halfWidth) };
			const int base1Y{ static_cast<int>(centerY + dirY * outerRadius + perpY * halfWidth) };
			const int base2X{ static_cast<int>(centerX + dirX * outerRadius - perpX * halfWidth) };
			const int base2Y{ static_cast<int>(centerY + dirY * outerRadius - perpY * halfWidth) };

			m_uiRenderer.drawTriangle(apexX, apexY, base1X, base1Y, base2X, base2Y, lineColor, true);
		}
	}
} // namespace game::system
