#include "InGameStatusView.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IStringConverter.h"
#include "core/utility/Color.h"
#include <algorithm>
#include <cstdio>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// パネルの位置とサイズ（右上・1080p基準）。左上のObjectiveViewと余白を揃える
	constexpr int PANEL_MARGIN{ 28 };
	constexpr int PANEL_WIDTH{ 250 };
	constexpr int PANEL_HEIGHT{ 104 };
	constexpr int PANEL_PADDING{ 20 };

	// パネル内の各要素の位置（パネル左上からの相対座標・1080p基準）
	constexpr int CAPTION_Y{ 14 };
	constexpr int CAPTION_FONT_SIZE{ 14 };
	constexpr int TIME_Y{ 40 };
	constexpr int TIME_FONT_SIZE{ 40 };

	// 難易度バッジ（右上の角に置く小さなチップ）
	constexpr int BADGE_Y{ 12 };
	constexpr int BADGE_HEIGHT{ 22 };
	constexpr int BADGE_PADDING_X{ 10 };
	constexpr int BADGE_RADIUS{ 4 };
	constexpr int BADGE_FONT_SIZE{ 14 };
	constexpr int BADGE_TEXT_OFFSET_Y{ 3 };

	// Hardのバッジ色（危険を示す赤）とNormalのバッジ色（Windows 11のアクセント）
	constexpr unsigned int BADGE_COLOR_HARD{ core::utility::Color::HUD_CRIT_RED };
	constexpr unsigned int BADGE_COLOR_NORMAL{ core::utility::Color::HUD_ACCENT };

	constexpr const char* MONO_FONT_NAME{ "Cascadia Mono SemiBold" };
	constexpr const char* UI_FONT_NAME{ "Noto Sans JP" };

	constexpr const char* CAPTION_TEXT{ "経過時間" };

	// 表示できる上限（99分59秒）。これを超えても桁が増えて崩れないよう頭打ちにする
	constexpr int MAX_DISPLAY_SECONDS{ 99 * 60 + 59 };
} // namespace

namespace game::ui::ingame
{
	InGameStatusView::InGameStatusView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::data::Difficulty difficulty)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_panel{ uiRenderer, screen }
	    , m_difficulty{ difficulty }
	    , m_captionText{ CAPTION_TEXT }
	{
		// DxLibの描画はShift_JISを期待する。ソース上のUTF-8をそのまま渡すと日本語が化けるため、
		// ここで一度だけ変換しておく（変換結果は毎フレーム同じなので描画時にはやらない）
		auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };
		if (!converter)
			return;

		m_captionText = converter->utf8ToShiftJis(m_captionText);
	}

	int InGameStatusView::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	void InGameStatusView::draw(float elapsedTime)
	{
		const int panelWidth{ scaled(PANEL_WIDTH) };
		const int panelHeight{ scaled(PANEL_HEIGHT) };
		const int panelX{ m_screen.getWidth() - scaled(PANEL_MARGIN) - panelWidth };
		const int panelY{ scaled(PANEL_MARGIN) };

		m_panel.draw(panelX, panelY, panelWidth, panelHeight);

		const int padding{ scaled(PANEL_PADDING) };

		m_uiRenderer.setFont(UI_FONT_NAME);
		m_uiRenderer.drawText(panelX + padding, panelY + scaled(CAPTION_Y), m_captionText.c_str(),
		    core::utility::Color::HUD_INK_FAINT, scaled(CAPTION_FONT_SIZE));

		// 難易度バッジはパネル右上の角に寄せる。文字幅ぶんだけ左へ戻して右端を揃える
		const char* difficultyText{ core::data::toText(m_difficulty).data() };
		const int badgeFontSize{ scaled(BADGE_FONT_SIZE) };
		const int badgeTextWidth{ m_uiRenderer.getTextWidth(difficultyText, badgeFontSize) };
		const int badgeWidth{ badgeTextWidth + scaled(BADGE_PADDING_X) * 2 };
		const int badgeX{ panelX + panelWidth - padding - badgeWidth };
		const int badgeY{ panelY + scaled(BADGE_Y) };
		const unsigned int badgeColor{ (m_difficulty == core::data::Difficulty::Hard)
			                               ? BADGE_COLOR_HARD
			                               : BADGE_COLOR_NORMAL };

		m_uiRenderer.drawRoundedBox(badgeX, badgeY, badgeWidth, scaled(BADGE_HEIGHT),
		    scaled(BADGE_RADIUS), badgeColor, true, 1);
		m_uiRenderer.drawText(badgeX + scaled(BADGE_PADDING_X),
		    badgeY + scaled(BADGE_TEXT_OFFSET_Y), difficultyText,
		    core::utility::Color::HUD_INK, badgeFontSize);
		m_uiRenderer.resetFont();

		// 経過時間は毎フレーム桁が動く。等幅で描いて数字の揺れを止める
		const int totalSeconds{ std::clamp(static_cast<int>(elapsedTime), 0, MAX_DISPLAY_SECONDS) };
		char timeText[8]{};
		std::snprintf(timeText, sizeof(timeText), "%02d:%02d", totalSeconds / 60, totalSeconds % 60);

		m_uiRenderer.setFont(MONO_FONT_NAME);
		m_uiRenderer.drawText(panelX + padding, panelY + scaled(TIME_Y), timeText,
		    core::utility::Color::HUD_INK, scaled(TIME_FONT_SIZE));
		m_uiRenderer.resetFont();
	}
} // namespace game::ui::ingame
