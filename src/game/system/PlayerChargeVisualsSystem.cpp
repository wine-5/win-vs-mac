#include "PlayerChargeVisualsSystem.h"
#include "game/component/PlayerChargeComponent.h"
#include "core/utility/Color.h"
#include <cmath>
#include <numbers>

namespace
{
	constexpr float ROTATION_SPEED{ 0.5f }; // 集中線の回転速度（ラジアン/秒）
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

		const int centerX{ m_screen.getWidth() / 2 };
		const int centerY{ m_screen.getHeight() / 2 };

		// 溜め率に応じて線の本数と長さが変わる
		const int lineCount{ static_cast<int>(8 + charge.m_chargeRate * 16) };    // 8〜24本
		const int maxLength{ static_cast<int>(300 + charge.m_chargeRate * 200) }; // 300〜500px
		const int lineThickness{ static_cast<int>(1 + charge.m_chargeRate * 2) }; // 1〜3px
		const unsigned int lineColor{ core::utility::Color::rgb(255, 200, 100) }; // オレンジ系

		// 放射状に集中線を描画（ゆっくり回転させて溜めの躍動感を出す）
		constexpr float PI{ static_cast<float>(std::numbers::pi) };
		const float baseAngle{ m_animationTime * ROTATION_SPEED };
		for (int i = 0; i < lineCount; ++i)
		{
			const float angle{ baseAngle + (2.0f * PI / lineCount) * i };
			const float dirX{ std::cos(angle) };
			const float dirY{ std::sin(angle) };

			// 中心から外側に向かって小さなボックスを連結し、線を表現する
			const int segments{ maxLength / 10 + 1 };
			for (int s = 0; s < segments; ++s)
			{
				const float t{ static_cast<float>(s) / segments };
				const int x{ centerX + static_cast<int>(dirX * maxLength * t) };
				const int y{ centerY + static_cast<int>(dirY * maxLength * t) };
				m_uiRenderer.drawBox(x, y, lineThickness, lineThickness, lineColor, true);
			}
		}
	}
} // namespace game::system
