#include "BossHUDView.h"
#include "core/constant/UI.h"
#include "core/utility/Color.h"
#include "game/component/ai/MacAIComponent.h"
#include "game/component/combat/HealthComponent.h"
#include <algorithm>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// パネルの位置とサイズ（上中央・1080p基準）
	constexpr int PANEL_MARGIN_TOP{ 28 };
	constexpr int PANEL_WIDTH{ 760 };
	constexpr int PANEL_HEIGHT{ 92 };
	constexpr int PANEL_PADDING{ 20 };

	// パネル内の各要素の位置（パネル左上からの相対座標・1080p基準）
	constexpr int NAME_Y{ 16 };
	constexpr int NAME_FONT_SIZE{ 24 };
	constexpr int BAR_Y{ 56 };
	constexpr int BAR_HEIGHT{ 18 };

	// フェーズを示すピル
	constexpr int PILL_Y{ 16 };
	constexpr int PILL_HEIGHT{ 26 };
	constexpr int PILL_PADDING{ 12 };
	constexpr int PILL_RADIUS{ 4 }; // Windows 11のコントロールの角丸
	constexpr int PILL_FONT_SIZE{ 14 };
	constexpr int PILL_FILL_ALPHA{ 46 };
	constexpr int PILL_BORDER_ALPHA{ 150 };

	constexpr int BAR_GROOVE_ALPHA{ 20 }; // バーの溝（白をごく薄く敷く）

	// ボスのHPバーは常に赤。残量で色を変えると「あと少し」が伝わりにくくなるうえ、
	// 覚醒フェーズの赤と競合して状態が読めなくなる
	constexpr unsigned int BAR_COLOR{ 0xFFE81123 };

	constexpr const char* BOSS_NAME{ "MacBook" };
	constexpr const char* PHASE_NORMAL_TEXT{ "PHASE 1" };
	constexpr const char* PHASE_AWAKENED_TEXT{ "PHASE 2 - AWAKENED" };
} // namespace

namespace game::ui::ingame
{
	BossHUDView::BossHUDView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::ecs::ComponentManager& componentManager)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_componentManager{ componentManager }
	    , m_panel{ uiRenderer, screen }
	{
	}

	int BossHUDView::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	void BossHUDView::draw(core::ecs::EntityId bossId)
	{
		if (bossId == core::ecs::INVALID_ENTITY_ID)
			return;
		if (!m_componentManager.has<component::combat::HealthComponent>(bossId))
			return;

		const auto& health{ m_componentManager.get<component::combat::HealthComponent>(bossId) };
		if (health.m_maxHp <= 0.0f)
			return;

		const int panelWidth{ scaled(PANEL_WIDTH) };
		const int panelHeight{ scaled(PANEL_HEIGHT) };
		// 幅はモニタのアスペクト比で変わるため、中央寄せは必ず実際の画面幅から求める
		const int panelX{ (m_screen.getWidth() - panelWidth) / 2 };
		const int panelY{ scaled(PANEL_MARGIN_TOP) };

		m_panel.draw(panelX, panelY, panelWidth, panelHeight);

		const int padding{ scaled(PANEL_PADDING) };

		m_uiRenderer.setFont(core::constant::ui::MONO_SEMIBOLD_FONT_NAME);
		m_uiRenderer.drawText(panelX + padding, panelY + scaled(NAME_Y), BOSS_NAME,
		    core::utility::Color::HUD_INK, scaled(NAME_FONT_SIZE));
		m_uiRenderer.resetFont();

		drawPhasePill(panelX + panelWidth - padding, panelY + scaled(PILL_Y), bossId);

		const float ratio{ std::clamp(health.m_currentHp / health.m_maxHp, 0.0f, 1.0f) };
		drawHealthBar(panelX + padding, panelY + scaled(BAR_Y),
		    panelWidth - padding * 2, scaled(BAR_HEIGHT), ratio);
	}

	void BossHUDView::drawPhasePill(int rightX, int y, core::ecs::EntityId bossId)
	{
		// フェーズを持たない敵がボス枠に入る可能性もあるため、無ければピルごと出さない
		if (!m_componentManager.has<component::ai::MacAIComponent>(bossId))
			return;

		const auto& ai{ m_componentManager.get<component::ai::MacAIComponent>(bossId) };
		const bool isAwakened{ ai.m_currentPhase == core::data::MacPhase::Awakened };
		const char* text{ isAwakened ? PHASE_AWAKENED_TEXT : PHASE_NORMAL_TEXT };

		// 覚醒したら赤、それまでは控えめなアクセント色。色の変化そのものが移行の合図になる
		const unsigned int color{ isAwakened ? core::utility::Color::HUD_CRIT_RED
			                                 : core::utility::Color::HUD_ACCENT };

		const int fontSize{ scaled(PILL_FONT_SIZE) };
		m_uiRenderer.setFont(core::constant::ui::MONO_SEMIBOLD_FONT_NAME);
		const int textWidth{ m_uiRenderer.getTextWidth(text, fontSize) };

		const int padding{ scaled(PILL_PADDING) };
		const int pillWidth{ textWidth + padding * 2 };
		const int pillHeight{ scaled(PILL_HEIGHT) };
		const int pillX{ rightX - pillWidth };
		const int radius{ scaled(PILL_RADIUS) };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, PILL_FILL_ALPHA);
		m_uiRenderer.drawRoundedBox(pillX, y, pillWidth, pillHeight, radius, color, true, 1);
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, PILL_BORDER_ALPHA);
		m_uiRenderer.drawRoundedBox(pillX, y, pillWidth, pillHeight, radius, color, false, 1);
		m_uiRenderer.resetBlendMode();

		// 文字は枠の中央に置く。フォントの実高さぶんを下げて縦位置を合わせる
		m_uiRenderer.drawText(pillX + padding, y + (pillHeight - fontSize) / 2, text, color, fontSize);
		m_uiRenderer.resetFont();
	}

	void BossHUDView::drawHealthBar(int x, int y, int width, int height, float ratio)
	{
		// 高さの半分を半径にすると、Windows 11のプログレスバーと同じピル形状になる
		const int radius{ height / 2 };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, BAR_GROOVE_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, core::utility::Color::WHITE, true, 1);
		m_uiRenderer.resetBlendMode();

		if (ratio <= 0.0f)
			return;

		// 幅が高さを下回るとピルが成立しないため、残量が僅かでも高さぶんは確保する
		const int fillWidth{ std::max(height, static_cast<int>(width * ratio)) };
		m_uiRenderer.drawRoundedBox(x, y, fillWidth, height, radius, BAR_COLOR, true, 1);
	}
} // namespace game::ui::ingame
