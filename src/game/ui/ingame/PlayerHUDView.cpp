#include "PlayerHUDView.h"
#include "core/constant/UI.h"
#include "core/utility/Color.h"
#include "game/component/combat/HealthComponent.h"
#include <algorithm>
#include <cstdio>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// パネルの位置とサイズ（左下・1080p基準）
	constexpr int PANEL_MARGIN{ 22 };
	constexpr int PANEL_WIDTH{ 270 };
	constexpr int PANEL_HEIGHT{ 66 };
	constexpr int PANEL_PADDING{ 14 };
	constexpr int PANEL_RADIUS{ 8 }; // Windows 11のウィンドウ・パネルの角丸

	// パネル内の各要素の位置（パネル左上からの相対座標・1080p基準）
	constexpr int LABEL_Y{ 12 };
	constexpr int LABEL_FONT_SIZE{ 13 };
	constexpr int VALUE_FONT_SIZE{ 12 };
	constexpr int BAR_Y{ 38 };
	constexpr int BAR_HEIGHT{ 12 };

	// パネルの塗りと枠。DxLibのブレンドはアルファ値を別途指定するため、色と不透明度を分けて持つ
	constexpr unsigned int PANEL_FILL_COLOR{ 0xFF0E1420 };
	constexpr int PANEL_FILL_ALPHA{ 184 }; // 約72%
	constexpr unsigned int PANEL_BORDER_COLOR{ 0xFF8CAAD2 };
	constexpr int PANEL_BORDER_ALPHA{ 46 }; // 約18%
	constexpr int BAR_GROOVE_ALPHA{ 20 };   // バーの溝（白をごく薄く敷く）

	// HP残量に応じたバーの色。Windows 11のプログレスバーに倣い単色で塗る
	constexpr unsigned int BAR_COLOR_HIGH{ 0xFF36D07B };
	constexpr unsigned int BAR_COLOR_MID{ 0xFFFFC83D };
	constexpr unsigned int BAR_COLOR_LOW{ 0xFFE81123 };
	constexpr float BAR_MID_THRESHOLD{ 0.5f };
	constexpr float BAR_LOW_THRESHOLD{ 0.2f };

	// フォント。数値・英字は等幅、日本語を含みうるラベルはNoto Sans JPで描く
	constexpr const char* MONO_FONT_NAME{ "Cascadia Mono" };
	constexpr const char* UI_FONT_NAME{ "Noto Sans JP" };

	constexpr const char* STATUS_LABEL{ "PLAYER STATUS" };
} // namespace

namespace game::ui::ingame
{
	PlayerHUDView::PlayerHUDView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::ecs::ComponentManager& componentManager)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_componentManager{ componentManager }
	{
	}

	int PlayerHUDView::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	void PlayerHUDView::draw(core::ecs::EntityId playerId)
	{
		if (!m_componentManager.has<component::combat::HealthComponent>(playerId))
			return;

		const auto& health{ m_componentManager.get<component::combat::HealthComponent>(playerId) };
		if (health.m_maxHp <= 0.0f)
			return;

		const int panelWidth{ scaled(PANEL_WIDTH) };
		const int panelHeight{ scaled(PANEL_HEIGHT) };
		const int panelX{ scaled(PANEL_MARGIN) };
		const int panelY{ m_screen.getHeight() - scaled(PANEL_MARGIN) - panelHeight };

		drawPanel(panelX, panelY, panelWidth, panelHeight);

		// 左に見出し、右にHPの実数値。数値は桁が動いても右端が揃うよう右寄せで置く
		const int padding{ scaled(PANEL_PADDING) };
		m_uiRenderer.setFont(UI_FONT_NAME);
		m_uiRenderer.drawText(panelX + padding, panelY + scaled(LABEL_Y), STATUS_LABEL,
		    core::utility::Color::HUD_INK, scaled(LABEL_FONT_SIZE));

		char hpText[32]{};
		std::snprintf(hpText, sizeof(hpText), "HP %d / %d",
		    static_cast<int>(health.m_currentHp), static_cast<int>(health.m_maxHp));

		m_uiRenderer.setFont(MONO_FONT_NAME);
		const int valueFontSize{ scaled(VALUE_FONT_SIZE) };
		const int valueWidth{ m_uiRenderer.getTextWidth(hpText, valueFontSize) };
		m_uiRenderer.drawText(panelX + panelWidth - padding - valueWidth, panelY + scaled(LABEL_Y),
		    hpText, core::utility::Color::HUD_INK, valueFontSize);
		m_uiRenderer.resetFont();

		// 0除算はmaxHpのチェックで防いでいる。回復過多などで1.0を超えても溝からはみ出さないよう丸める
		const float ratio{ std::clamp(health.m_currentHp / health.m_maxHp, 0.0f, 1.0f) };
		drawHealthBar(panelX + padding, panelY + scaled(BAR_Y),
		    panelWidth - padding * 2, scaled(BAR_HEIGHT), ratio);
	}

	void PlayerHUDView::drawPanel(int x, int y, int width, int height)
	{
		const int radius{ scaled(PANEL_RADIUS) };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, PANEL_FILL_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, PANEL_FILL_COLOR, true, 1);

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, PANEL_BORDER_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, PANEL_BORDER_COLOR, false, 1);

		m_uiRenderer.resetBlendMode();
	}

	void PlayerHUDView::drawHealthBar(int x, int y, int width, int height, float ratio)
	{
		// 高さの半分を半径にすると、Windows 11のプログレスバーと同じピル形状になる
		const int radius{ height / 2 };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, BAR_GROOVE_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, core::utility::Color::WHITE, true, 1);
		m_uiRenderer.resetBlendMode();

		const int fillWidth{ static_cast<int>(width * ratio) };
		// 残りわずかなときに潰れて見えなくなるのを防ぐため、ピルが成立する最小幅を確保する
		if (fillWidth < height)
			return;

		unsigned int fillColor{ BAR_COLOR_HIGH };
		if (ratio <= BAR_LOW_THRESHOLD)
			fillColor = BAR_COLOR_LOW;
		else if (ratio <= BAR_MID_THRESHOLD)
			fillColor = BAR_COLOR_MID;

		m_uiRenderer.drawRoundedBox(x, y, fillWidth, height, radius, fillColor, true, 1);
	}
} // namespace game::ui::ingame
