#include "ObjectiveView.h"
#include "core/base/ServiceLocator.h"
#include "core/constant/UI.h"
#include "core/interface/IStringConverter.h"
#include "core/utility/Color.h"
#include <algorithm>
#include <cstdio>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// パネルの位置とサイズ（左上・1080p基準）
	constexpr int PANEL_MARGIN{ 28 };
	constexpr int PANEL_WIDTH{ 330 };
	constexpr int PANEL_HEIGHT{ 104 };
	constexpr int PANEL_PADDING{ 20 };
	constexpr int PANEL_RADIUS{ 8 }; // Windows 11のウィンドウ・パネルの角丸

	// パネル内の各要素の位置（パネル左上からの相対座標・1080p基準）
	constexpr int CAPTION_Y{ 14 };
	constexpr int CAPTION_FONT_SIZE{ 14 };
	constexpr int COUNT_Y{ 40 };
	constexpr int COUNT_FONT_SIZE{ 44 };
	constexpr int DETAIL_Y{ 58 };
	constexpr int DETAIL_FONT_SIZE{ 17 };
	constexpr int COUNT_DETAIL_GAP{ 12 };
	constexpr int BOSS_LABEL_Y{ 48 };
	constexpr int BOSS_FONT_SIZE{ 26 };

	// パネルの塗りと枠。色と不透明度を分けて持つ（DxLibのブレンドはアルファを別途指定するため）
	constexpr unsigned int PANEL_FILL_COLOR{ 0xFF0E1420 };
	constexpr int PANEL_FILL_ALPHA{ 184 }; // 約72%
	constexpr unsigned int PANEL_BORDER_COLOR{ 0xFF8CAAD2 };
	constexpr int PANEL_BORDER_ALPHA{ 46 }; // 約18%

	constexpr const char* MONO_FONT_NAME{ "Cascadia Mono SemiBold" };
	constexpr const char* UI_FONT_NAME{ "Noto Sans JP" };

	constexpr const char* CAPTION_TEXT{ "OBJECTIVE" };
	constexpr const char* DETAIL_TEXT{ "体 倒すと Mac が出現" };
	constexpr const char* BOSS_TEXT{ "Mac を破壊せよ" };
} // namespace

namespace game::ui::ingame
{
	ObjectiveView::ObjectiveView(core::iface::IUIRenderer& uiRenderer, core::iface::IScreen& screen)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_detailText{ DETAIL_TEXT }
	    , m_bossText{ BOSS_TEXT }
	{
		// DxLibの描画はShift_JISを期待する。ソース上のUTF-8をそのまま渡すと日本語が化けるため、
		// ここで一度だけ変換しておく（変換結果は毎フレーム同じなので描画時にはやらない）
		auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };
		if (!converter)
			return;

		m_detailText = converter->utf8ToShiftJis(m_detailText);
		m_bossText = converter->utf8ToShiftJis(m_bossText);
	}

	int ObjectiveView::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	void ObjectiveView::draw(int remainingEnemyCount, bool isBossAppeared)
	{
		const int panelX{ scaled(PANEL_MARGIN) };
		const int panelY{ scaled(PANEL_MARGIN) };
		const int panelWidth{ scaled(PANEL_WIDTH) };
		const int panelHeight{ scaled(PANEL_HEIGHT) };

		drawPanel(panelX, panelY, panelWidth, panelHeight);

		const int padding{ scaled(PANEL_PADDING) };

		m_uiRenderer.setFont(UI_FONT_NAME);
		m_uiRenderer.drawText(panelX + padding, panelY + scaled(CAPTION_Y), CAPTION_TEXT,
		    core::utility::Color::HUD_INK_FAINT, scaled(CAPTION_FONT_SIZE));

		// ボスが出たら残り数は無意味になる。目標そのものを討伐へ差し替える
		if (isBossAppeared)
		{
			m_uiRenderer.drawText(panelX + padding, panelY + scaled(BOSS_LABEL_Y), m_bossText.c_str(),
			    core::utility::Color::HUD_CRIT_RED, scaled(BOSS_FONT_SIZE));
			m_uiRenderer.resetFont();
			return;
		}
		m_uiRenderer.resetFont();

		// 残り数は視線を最初に落とす場所なので、等幅の大きな数字で単独で見せる。
		// 2桁ぶんの幅を常に確保して、数が減っても後ろの文言が左右に動かないようにする
		char countText[8]{};
		std::snprintf(countText, sizeof(countText), "%02d", std::max(0, remainingEnemyCount));

		m_uiRenderer.setFont(MONO_FONT_NAME);
		const int countFontSize{ scaled(COUNT_FONT_SIZE) };
		m_uiRenderer.drawText(panelX + padding, panelY + scaled(COUNT_Y), countText,
		    core::utility::Color::HUD_INK, countFontSize);
		const int countWidth{ m_uiRenderer.getTextWidth(countText, countFontSize) };
		m_uiRenderer.resetFont();

		m_uiRenderer.setFont(UI_FONT_NAME);
		m_uiRenderer.drawText(panelX + padding + countWidth + scaled(COUNT_DETAIL_GAP),
		    panelY + scaled(DETAIL_Y), m_detailText.c_str(),
		    core::utility::Color::HUD_INK_FAINT, scaled(DETAIL_FONT_SIZE));
		m_uiRenderer.resetFont();
	}

	void ObjectiveView::drawPanel(int x, int y, int width, int height)
	{
		const int radius{ scaled(PANEL_RADIUS) };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, PANEL_FILL_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, PANEL_FILL_COLOR, true, 1);

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, PANEL_BORDER_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, PANEL_BORDER_COLOR, false, 1);

		m_uiRenderer.resetBlendMode();
	}
} // namespace game::ui::ingame
