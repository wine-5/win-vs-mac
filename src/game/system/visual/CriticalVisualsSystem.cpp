#include "CriticalVisualsSystem.h"
#include "core/constant/UI.h"
#include "core/utility/Color.h"
#include "core/utility/MathConstants.h"
#include "game/component/TagComponent.h"
#include "game/constant/Tag.h"
#include <cmath>

namespace
{
	// 演出時間（秒）。ヒットストップ（0.10秒）を少し超える程度に留め、
	// 止まりが明けたら画面がすぐ元通りになるようにする
	constexpr float BURST_DURATION{ 0.18f };

	constexpr float OUTER_RADIUS_MARGIN{ 1.05f }; // 画面対角の半分に対する余裕（線の根本を画面外に出す）
	constexpr int LINE_COUNT{ 20 };               // 集中線の本数

	// 線の先端（中央側）の位置。始まりは中央寄り、終わりは外へ逃げていく（画面高さ比）。
	// 弾けた線が外へ抜けていくように見せるため、時間とともに広げる
	constexpr float INNER_RADIUS_START{ 0.22f };
	constexpr float INNER_RADIUS_END{ 0.62f };

	constexpr float WEDGE_HALF_WIDTH_MIN{ 0.004f };  // くさびの根本半幅の最小（画面高さ比）
	constexpr float WEDGE_HALF_WIDTH_RAND{ 0.010f }; // くさびの根本半幅の乱数幅（画面高さ比）

	// 半透明合成に渡す不透明度の最大値
	constexpr float ALPHA_MAX{ 255.0f };

	/**
	 * @brief 整数の種から0.0〜1.0の疑似乱数を返す（同じ種なら常に同じ値になる決定的な乱数）
	 *
	 * 溜め演出（PlayerChargeVisualsSystem）と同じ手法。1発ぶんの線の並びを
	 * 演出中ずっと固定したいので、状態を持つ乱数生成器ではなく種から引く
	 * @param seed 乱数の種
	 * @return 0.0〜1.0の値
	 */
	float rand01(int seed)
	{
		const float value{ std::sin(static_cast<float>(seed) * 12.9898f) * 43758.5453f };
		return value - std::floor(value);
	}
} // namespace

namespace game::system::visual
{
	CriticalVisualsSystem::CriticalVisualsSystem(core::ecs::ComponentManager& componentManager,
	    core::base::EventBus& eventBus,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::ecs::EntityId playerId)
	    : m_componentManager{ componentManager }
	    , m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_playerId{ playerId }
	{
		m_subscriptions.push_back(eventBus.subscribe<event::AttackHitEvent>(
		    [this](const event::AttackHitEvent& e)
		    { onAttackHit(e); }));
	}

	void CriticalVisualsSystem::update(float deltaTime)
	{
		// 経過時間は壁時計で測るため、ここでは何もしない（ヘッダのコメント参照）
		(void)deltaTime;
	}

	void CriticalVisualsSystem::onAttackHit(const event::AttackHitEvent& event)
	{
		if (!event.m_isCritical)
			return;

		// プレイヤーが与えたクリティカルにだけ反応する。被弾側で画面を覆うと
		// 「自分がやられた」演出と紛らわしいうえ、視界を塞いで理不尽になる
		const auto* targetTag{ m_componentManager.tryGet<component::TagComponent>(event.m_targetId) };
		if (targetTag == nullptr || targetTag->m_tag != constant::Tag::Enemy)
			return;

		// 発生ごとに線の並びを変える必要はない。開始時刻だけ更新して重ね掛けを避ける
		m_startTime = std::chrono::steady_clock::now();
		m_isActive = true;
	}

	void CriticalVisualsSystem::draw()
	{
		if (!m_isActive)
			return;

		const float elapsed{ std::chrono::duration<float>(
			std::chrono::steady_clock::now() - m_startTime)
			    .count() };
		if (elapsed >= BURST_DURATION)
		{
			m_isActive = false;
			return;
		}

		const float progress{ elapsed / BURST_DURATION };

		const float screenW{ static_cast<float>(m_screen.getWidth()) };
		const float screenH{ static_cast<float>(m_screen.getHeight()) };
		const float centerX{ screenW * 0.5f };
		const float centerY{ screenH * 0.5f };

		// 線の根本は画面の外から生やし、必ず画面端まで届かせる
		const float outerRadius{ std::sqrt(screenW * screenW + screenH * screenH) * 0.5f * OUTER_RADIUS_MARGIN };

		// 先端が外へ逃げていく（減速しながら抜けると、弾けて散ったように見える）
		const float eased{ 1.0f - (1.0f - progress) * (1.0f - progress) };
		const float innerRadius{ screenH * (INNER_RADIUS_START + (INNER_RADIUS_END - INNER_RADIUS_START) * eased) };

		// 出た瞬間が最も濃く、あとは一直線に薄れる
		const int alphaParam{ static_cast<int>(ALPHA_MAX * (1.0f - progress)) };
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alphaParam);

		const float angleStep{ core::utility::TWO_PI / LINE_COUNT };

		for (int i = 0; i < LINE_COUNT; ++i)
		{
			// 等間隔を基準に、角度・先端位置・太さを乱して手描きらしくする
			const float angle{ angleStep * i + (rand01(i) - 0.5f) * angleStep * 0.8f };
			const float apexRadius{ innerRadius * (1.0f + (rand01(i + 1) - 0.5f) * 0.3f) };
			const float halfWidth{ screenH * (WEDGE_HALF_WIDTH_MIN + rand01(i + 2) * WEDGE_HALF_WIDTH_RAND) };

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

			m_uiRenderer.drawTriangle(apexX, apexY, base1X, base1Y, base2X, base2Y,
			    core::utility::Color::HUD_CRITICAL_ORANGE, true);
		}

		m_uiRenderer.resetBlendMode();
	}
} // namespace game::system::visual
