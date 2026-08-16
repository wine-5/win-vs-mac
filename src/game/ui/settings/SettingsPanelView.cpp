#include "SettingsPanelView.h"
#include "core/base/ServiceLocator.h"
#include "core/constant/UI.h"
#include "core/interface/IStringConverter.h"
#include "core/utility/Color.h"
#include <algorithm>
#include <string>

namespace
{
	using Color = core::utility::Color;

	// 背景オーバーレイの暗さ（ポーズメニューと同じ値にして、開き方で明るさが変わらないようにする）
	constexpr int OVERLAY_ALPHA{ 160 };

	// ウィンドウの大きさ（画面に対する比率）
	constexpr float PANEL_WIDTH_RATIO{ 0.58f };
	constexpr float PANEL_HEIGHT_RATIO{ 0.70f };

	// 左ナビの幅（ウィンドウ幅に対する比率）
	constexpr float NAV_WIDTH_RATIO{ 0.27f };

	// 以下は基準サイズ（ウィンドウ高さ660px）でのピクセル数。実際の描画では scaled() を通す。
	// Windows 11 の実物の寸法をそのまま書いておき、換算はまとめて1か所で行う
	constexpr float BASE_PANEL_HEIGHT{ 660.0f };
	constexpr float TITLE_BAR_HEIGHT{ 48.0f };
	constexpr float CONTENT_PADDING_LEFT{ 32.0f };
	constexpr float CONTENT_PADDING_RIGHT{ 36.0f };
	constexpr float CONTENT_PADDING_TOP{ 24.0f };
	constexpr float ROW_HEIGHT{ 62.0f };
	constexpr float ROW_PADDING_X{ 16.0f };
	constexpr float PANEL_RADIUS{ 8.0f };
	constexpr float CARD_RADIUS{ 4.0f };
	constexpr float ICON_COLUMN_WIDTH{ 36.0f }; // 行の左端に空けておくアイコン用の幅
	constexpr float SLIDER_WIDTH{ 200.0f };
	constexpr float SLIDER_TRACK_HEIGHT{ 4.0f };
	constexpr float SLIDER_THUMB_RADIUS{ 10.0f };
	constexpr float SLIDER_THUMB_INNER_RADIUS{ 6.0f };
	constexpr float VALUE_WIDTH{ 34.0f };
	constexpr float TOGGLE_WIDTH{ 40.0f };
	constexpr float TOGGLE_HEIGHT{ 20.0f };
	constexpr float TOGGLE_KNOB_RADIUS{ 6.0f };
	constexpr float BUTTON_WIDTH{ 84.0f };
	constexpr float BUTTON_HEIGHT{ 32.0f };
	constexpr float NAV_ITEM_HEIGHT{ 36.0f };
	constexpr float NAV_PILL_WIDTH{ 3.0f };
	constexpr float NAV_PILL_HEIGHT{ 16.0f };
	constexpr float AVATAR_RADIUS{ 18.0f };
	constexpr float SECTION_LABEL_GAP{ 8.0f }; // 見出しとカードの間隔
	constexpr float SECTION_TOP_GAP{ 24.0f };  // カードと次の見出しの間隔
	constexpr float FOCUS_RING_THICKNESS{ 2.0f };

	// フォントサイズ（基準サイズでのピクセル数）
	constexpr float FONT_PAGE_TITLE{ 28.0f };
	constexpr float FONT_SECTION{ 15.0f };
	constexpr float FONT_ROW_TITLE{ 15.0f };
	constexpr float FONT_ROW_SUB{ 13.0f };
	constexpr float FONT_NAV{ 14.0f };
	constexpr float FONT_TITLE_BAR{ 13.0f };
	constexpr float FONT_ACCOUNT_NAME{ 14.0f };
	constexpr float FONT_ACCOUNT_SUB{ 12.0f };
} // namespace

namespace game::ui::settings
{
	SettingsPanelView::SettingsPanelView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	{
	}

	void SettingsPanelView::draw(const core::data::GameSettings& settings, SettingsPage page,
	    int focusIndex, bool showFocus)
	{
		updateLayout();

		// 左ナビの番号ぶんを引いて、ページ内での行番号にする（負なら行は選択されていない）
		const int selectedRow{ focusIndex - PAGE_COUNT };

		// 背後のシーンを半透明の黒で沈めてから、その上にウィンドウを置く
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, OVERLAY_ALPHA);
		m_uiRenderer.drawBox(0, 0, m_screen.getWidth(), m_screen.getHeight(), Color::BLACK, true);
		m_uiRenderer.resetBlendMode();

		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);

		drawWindow();
		drawTitleBar();
		drawNav(page, focusIndex, showFocus);

		if (page == SettingsPage::Sound)
			drawSoundPage(settings.m_audio, selectedRow, showFocus);
		else
			drawControlPage(settings.m_control, selectedRow, showFocus);

		m_uiRenderer.resetFont();
	}

	void SettingsPanelView::updateLayout()
	{
		const int screenWidth{ m_screen.getWidth() };
		const int screenHeight{ m_screen.getHeight() };

		m_panelWidth = static_cast<int>(screenWidth * PANEL_WIDTH_RATIO);
		m_panelHeight = static_cast<int>(screenHeight * PANEL_HEIGHT_RATIO);
		m_panelX = (screenWidth - m_panelWidth) / 2;
		m_panelY = (screenHeight - m_panelHeight) / 2;

		m_scale = m_panelHeight / BASE_PANEL_HEIGHT;

		m_titleBarHeight = scaled(TITLE_BAR_HEIGHT);
		m_navWidth = static_cast<int>(m_panelWidth * NAV_WIDTH_RATIO);
		m_contentX = m_panelX + m_navWidth + scaled(CONTENT_PADDING_LEFT);
		m_contentWidth = m_panelWidth - m_navWidth - scaled(CONTENT_PADDING_LEFT) - scaled(CONTENT_PADDING_RIGHT);
		m_contentTop = m_panelY + m_titleBarHeight + scaled(CONTENT_PADDING_TOP);
		m_rowHeight = scaled(ROW_HEIGHT);
	}

	int SettingsPanelView::scaled(float basePixels) const noexcept
	{
		return std::max(1, static_cast<int>(basePixels * m_scale));
	}

	void SettingsPanelView::drawWindow() const
	{
		const int radius{ scaled(PANEL_RADIUS) };

		// ウィンドウの地（左ナビ側の色）
		m_uiRenderer.drawRoundedBox(m_panelX, m_panelY, m_panelWidth, m_panelHeight,
		    radius, Color::SETTINGS_WINDOW_BG, true, 1);

		// 右のコンテンツ面。ウィンドウの右下までを覆い、角丸をウィンドウと共有する
		const int contentLeft{ m_panelX + m_navWidth };
		const int contentTop{ m_panelY + m_titleBarHeight };
		m_uiRenderer.drawRoundedBox(contentLeft, contentTop,
		    m_panelWidth - m_navWidth, m_panelHeight - m_titleBarHeight,
		    radius, Color::SETTINGS_CONTENT_BG, true, 1);

		// ウィンドウの外枠
		m_uiRenderer.drawRoundedBox(m_panelX, m_panelY, m_panelWidth, m_panelHeight,
		    radius, Color::SETTINGS_STROKE, false, 1);
	}

	void SettingsPanelView::drawTitleBar() const
	{
		const int fontSize{ scaled(FONT_TITLE_BAR) };
		const std::string title{ toDrawable("設定") };

		m_uiRenderer.drawText(m_panelX + scaled(CONTENT_PADDING_LEFT),
		    m_panelY + (m_titleBarHeight - fontSize) / 2,
		    title.c_str(), Color::SETTINGS_TEXT, fontSize);

		// 閉じるボタン（×）。線2本で描く
		const int closeCenterX{ m_panelX + m_panelWidth - scaled(28.0f) };
		const int closeCenterY{ m_panelY + m_titleBarHeight / 2 };
		const int arm{ scaled(5.0f) };
		m_uiRenderer.drawLine(closeCenterX - arm, closeCenterY - arm,
		    closeCenterX + arm, closeCenterY + arm, Color::SETTINGS_TEXT, 1);
		m_uiRenderer.drawLine(closeCenterX + arm, closeCenterY - arm,
		    closeCenterX - arm, closeCenterY + arm, Color::SETTINGS_TEXT, 1);
	}

	void SettingsPanelView::drawNav(SettingsPage page, int focusIndex, bool showFocus) const
	{
		const int navLeft{ m_panelX + scaled(12.0f) };
		const int navWidth{ m_navWidth - scaled(16.0f) };

		// ── アカウント行（この世界のプレイヤーが何者かを一行で見せる） ──
		const int avatarRadius{ scaled(AVATAR_RADIUS) };
		const int avatarCenterX{ navLeft + scaled(12.0f) + avatarRadius };
		const int avatarCenterY{ m_panelY + m_titleBarHeight + scaled(20.0f) + avatarRadius };

		m_uiRenderer.drawCircle(avatarCenterX, avatarCenterY, avatarRadius, Color::SETTINGS_CARD, true, 1);
		m_uiRenderer.drawCircle(avatarCenterX, avatarCenterY, avatarRadius, Color::SETTINGS_STROKE, false, 1);

		// 人型（頭＋肩）。円の内側に収まる大きさにしてはみ出させない
		m_uiRenderer.drawCircle(avatarCenterX, avatarCenterY - scaled(5.0f), scaled(6.0f),
		    Color::SETTINGS_TEXT_TERTIARY, true, 1);
		const int shoulderTop{ avatarCenterY + scaled(3.0f) };
		const int shoulderBottom{ avatarCenterY + scaled(11.0f) };
		const int shoulderTopHalf{ scaled(5.0f) };
		const int shoulderBottomHalf{ scaled(9.0f) };
		m_uiRenderer.drawTriangle(avatarCenterX - shoulderTopHalf, shoulderTop,
		    avatarCenterX + shoulderTopHalf, shoulderTop,
		    avatarCenterX + shoulderBottomHalf, shoulderBottom, Color::SETTINGS_TEXT_TERTIARY, true);
		m_uiRenderer.drawTriangle(avatarCenterX - shoulderTopHalf, shoulderTop,
		    avatarCenterX + shoulderBottomHalf, shoulderBottom,
		    avatarCenterX - shoulderBottomHalf, shoulderBottom, Color::SETTINGS_TEXT_TERTIARY, true);

		const int nameFontSize{ scaled(FONT_ACCOUNT_NAME) };
		const int subFontSize{ scaled(FONT_ACCOUNT_SUB) };
		const int textX{ avatarCenterX + avatarRadius + scaled(12.0f) };
		const std::string accountName{ toDrawable("セキュリティ エージェント") };

		m_uiRenderer.drawText(textX, avatarCenterY - nameFontSize - scaled(1.0f),
		    accountName.c_str(), Color::SETTINGS_TEXT, nameFontSize);
		m_uiRenderer.drawText(textX, avatarCenterY + scaled(2.0f),
		    "WIN-VS-MAC\\agent", Color::SETTINGS_TEXT_TERTIARY, subFontSize);

		// ── ナビ項目 ──
		const char* labels[PAGE_COUNT]{ "サウンド", "操作" };
		const int itemHeight{ scaled(NAV_ITEM_HEIGHT) };
		const int itemFontSize{ scaled(FONT_NAV) };
		int itemY{ avatarCenterY + avatarRadius + scaled(20.0f) };

		for (int i{ 0 }; i < PAGE_COUNT; ++i)
		{
			const bool isActive{ i == static_cast<int>(page) };

			if (isActive)
			{
				m_uiRenderer.drawRoundedBox(navLeft, itemY, navWidth, itemHeight,
				    scaled(5.0f), Color::SETTINGS_NAV_SELECTED, true, 1);

				// 選択中を示す左端のアクセントピル
				const int pillHeight{ scaled(NAV_PILL_HEIGHT) };
				m_uiRenderer.drawRoundedBox(navLeft, itemY + (itemHeight - pillHeight) / 2,
				    scaled(NAV_PILL_WIDTH), pillHeight, scaled(2.0f), Color::SETTINGS_ACCENT, true, 1);
			}

			// 操作の対象が左ナビにあるときだけ枠を出す（行を触っている間は出さない）
			if (i == focusIndex && showFocus)
				drawSelection(navLeft, itemY, navWidth, itemHeight, true);

			const std::string label{ toDrawable(labels[i]) };
			m_uiRenderer.drawText(navLeft + scaled(36.0f), itemY + (itemHeight - itemFontSize) / 2,
			    label.c_str(), Color::SETTINGS_TEXT, itemFontSize);

			itemY += itemHeight + scaled(2.0f);
		}
	}

	void SettingsPanelView::drawSoundPage(const core::data::AudioSettings& audio,
	    int selectedRow, bool showFocus) const
	{
		using Audio = core::data::AudioSettings;

		const int titleFontSize{ scaled(FONT_PAGE_TITLE) };
		const std::string pageTitle{ toDrawable("サウンド") };
		m_uiRenderer.drawText(m_contentX, m_contentTop, pageTitle.c_str(), Color::SETTINGS_TEXT, titleFontSize);

		int y{ m_contentTop + titleFontSize + scaled(SECTION_TOP_GAP) };

		// 出力（Windows の「音量」に当たる。ここがマスター）
		y = drawSectionLabel(y, "出力");
		y = drawCard(y, 1);
		drawRow(y, "音量", "ゲーム全体の音量", ControlKind::Slider,
		    audio.m_master, 0, Audio::MAX_LEVEL,
		    selectedRow == static_cast<int>(SoundRow::Master), showFocus);
		y += m_rowHeight + scaled(SECTION_TOP_GAP);

		// 音量ミキサー（Windows のアプリ別音量。BGM と 効果音 をアプリのように並べる）
		y = drawSectionLabel(y, "音量ミキサー");
		y = drawCard(y, 2);
		drawRow(y, "BGM", "WinVsMac.exe - Music", ControlKind::Slider,
		    audio.m_bgm, 0, Audio::MAX_LEVEL,
		    selectedRow == static_cast<int>(SoundRow::Bgm), showFocus);
		y += m_rowHeight;
		drawRow(y, "効果音", "WinVsMac.exe - Sound Effects", ControlKind::Slider,
		    audio.m_se, 0, Audio::MAX_LEVEL,
		    selectedRow == static_cast<int>(SoundRow::Se), showFocus);
		y += m_rowHeight + scaled(SECTION_TOP_GAP);

		y = drawSectionLabel(y, "詳細設定");
		y = drawCard(y, 1);
		drawRow(y, "音量を既定値に戻す", "音量 100 / BGM 100 / 効果音 100", ControlKind::Button,
		    0, 0, 0, selectedRow == static_cast<int>(SoundRow::Reset), showFocus);
	}

	void SettingsPanelView::drawControlPage(const core::data::ControlSettings& control,
	    int selectedRow, bool showFocus) const
	{
		using Control = core::data::ControlSettings;

		const int titleFontSize{ scaled(FONT_PAGE_TITLE) };
		const std::string pageTitle{ toDrawable("操作") };
		m_uiRenderer.drawText(m_contentX, m_contentTop, pageTitle.c_str(), Color::SETTINGS_TEXT, titleFontSize);

		int y{ m_contentTop + titleFontSize + scaled(SECTION_TOP_GAP) };

		y = drawSectionLabel(y, "マウス");
		y = drawCard(y, 2);
		drawRow(y, "カメラ感度", "マウスを動かしたときにカメラが回る速さ", ControlKind::Slider,
		    control.m_sensitivity, Control::MIN_SENSITIVITY, Control::MAX_SENSITIVITY,
		    selectedRow == static_cast<int>(ControlRow::Sensitivity), showFocus);
		y += m_rowHeight;
		drawRow(y, "Y軸を反転する", "マウスを下に動かすとカメラが上を向きます", ControlKind::Toggle,
		    control.m_invertY ? 1 : 0, 0, 1,
		    selectedRow == static_cast<int>(ControlRow::InvertY), showFocus);
		y += m_rowHeight + scaled(SECTION_TOP_GAP);

		y = drawSectionLabel(y, "カメラ");
		y = drawCard(y, 1);
		drawRow(y, "画面の揺れ", "被弾やボス演出での揺れの強さ。酔いやすい場合は下げてください",
		    ControlKind::Slider, control.m_screenShake, 0, Control::MAX_SHAKE,
		    selectedRow == static_cast<int>(ControlRow::Shake), showFocus);
		y += m_rowHeight + scaled(SECTION_TOP_GAP);

		y = drawSectionLabel(y, "詳細設定");
		y = drawCard(y, 1);
		drawRow(y, "操作を既定値に戻す", "カメラ感度 5 / Y軸反転 オフ / 画面の揺れ 100",
		    ControlKind::Button, 0, 0, 0,
		    selectedRow == static_cast<int>(ControlRow::Reset), showFocus);
	}

	void SettingsPanelView::drawSelection(int x, int y, int width, int height, bool showFocus) const
	{
		const int radius{ scaled(CARD_RADIUS) };
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, Color::SETTINGS_CARD_HOVER, true, 1);

		// Fluent のフォーカスは「外に白い線、内に暗い線」の二重リング。
		// マウスで触っている間は出さないので、キーボード・パッド操作中だけ描く
		if (!showFocus)
			return;

		const int thickness{ scaled(FOCUS_RING_THICKNESS) };
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, Color::SETTINGS_TEXT, false, thickness);
		m_uiRenderer.drawRoundedBox(x + thickness, y + thickness,
		    width - thickness * 2, height - thickness * 2,
		    radius, Color::SETTINGS_WINDOW_BG, false, 1);
	}

	int SettingsPanelView::drawSectionLabel(int y, const char* label) const
	{
		const int fontSize{ scaled(FONT_SECTION) };
		const std::string text{ toDrawable(label) };
		m_uiRenderer.drawText(m_contentX, y, text.c_str(), Color::SETTINGS_TEXT, fontSize);

		return y + fontSize + scaled(SECTION_LABEL_GAP);
	}

	int SettingsPanelView::drawCard(int y, int rowCount) const
	{
		const int height{ m_rowHeight * rowCount };
		const int radius{ scaled(CARD_RADIUS) };

		m_uiRenderer.drawRoundedBox(m_contentX, y, m_contentWidth, height, radius, Color::SETTINGS_CARD, true, 1);
		m_uiRenderer.drawRoundedBox(m_contentX, y, m_contentWidth, height, radius, Color::SETTINGS_STROKE, false, 1);

		// 行と行の区切り線（カードの中だけに引く）
		for (int i{ 1 }; i < rowCount; ++i)
		{
			m_uiRenderer.drawBox(m_contentX + 1, y + m_rowHeight * i, m_contentWidth - 2, 1,
			    Color::SETTINGS_STROKE, true);
		}

		return y;
	}

	void SettingsPanelView::drawRow(int y, const char* title, const char* sub, ControlKind kind,
	    int value, int minValue, int maxValue, bool isSelected, bool showFocus) const
	{
		const int centerY{ y + m_rowHeight / 2 };
		const int radius{ scaled(CARD_RADIUS) };

		if (isSelected)
			drawSelection(m_contentX + 1, y + 1, m_contentWidth - 2, m_rowHeight - 2, showFocus);

		// 見出しと説明。アイコン用の幅を左に空けてある（アイコンは別途描き足す）
		const int textX{ m_contentX + scaled(ROW_PADDING_X) + scaled(ICON_COLUMN_WIDTH) };
		const int titleFontSize{ scaled(FONT_ROW_TITLE) };
		const int subFontSize{ scaled(FONT_ROW_SUB) };
		const std::string titleText{ toDrawable(title) };
		const bool hasSub{ sub != nullptr && sub[0] != '\0' };

		if (hasSub)
		{
			const std::string subText{ toDrawable(sub) };
			m_uiRenderer.drawText(textX, centerY - titleFontSize - scaled(1.0f),
			    titleText.c_str(), Color::SETTINGS_TEXT, titleFontSize);
			m_uiRenderer.drawText(textX, centerY + scaled(2.0f),
			    subText.c_str(), Color::SETTINGS_TEXT_TERTIARY, subFontSize);
		}
		else
		{
			m_uiRenderer.drawText(textX, centerY - titleFontSize / 2,
			    titleText.c_str(), Color::SETTINGS_TEXT, titleFontSize);
		}

		const int right{ m_contentX + m_contentWidth - scaled(ROW_PADDING_X) };
		switch (kind)
		{
		case ControlKind::Slider: drawSlider(right, centerY, value, minValue, maxValue); break;
		case ControlKind::Toggle: drawToggle(right, centerY, value != 0); break;
		case ControlKind::Button: drawResetButton(right, centerY); break;
		}
	}

	void SettingsPanelView::drawSlider(int right, int centerY, int value, int minValue, int maxValue) const
	{
		// 数値はスライダーの右に置く。桁数で位置がずれないよう右揃えにする
		const int fontSize{ scaled(FONT_ROW_TITLE) };
		const std::string valueText{ std::to_string(value) };
		const int valueWidth{ m_uiRenderer.getTextWidth(valueText.c_str(), fontSize) };
		m_uiRenderer.drawText(right - valueWidth, centerY - fontSize / 2,
		    valueText.c_str(), Color::SETTINGS_TEXT_SECONDARY, fontSize);

		const int sliderRight{ right - scaled(VALUE_WIDTH) - scaled(14.0f) };
		const int sliderWidth{ scaled(SLIDER_WIDTH) };
		const int sliderLeft{ sliderRight - sliderWidth };

		const int trackHeight{ scaled(SLIDER_TRACK_HEIGHT) };
		const int trackY{ centerY - trackHeight / 2 };
		const int trackRadius{ std::max(1, trackHeight / 2) };

		// 未到達部分（右側）
		m_uiRenderer.drawRoundedBox(sliderLeft, trackY, sliderWidth, trackHeight,
		    trackRadius, Color::SETTINGS_TRACK, true, 1);

		// 到達部分（左側）。値の割合ぶんだけアクセント色で塗る
		const int range{ std::max(1, maxValue - minValue) };
		const float ratio{ static_cast<float>(std::clamp(value, minValue, maxValue) - minValue) / range };
		const int filledWidth{ static_cast<int>(sliderWidth * ratio) };
		if (filledWidth > 0)
		{
			m_uiRenderer.drawRoundedBox(sliderLeft, trackY, filledWidth, trackHeight,
			    trackRadius, Color::SETTINGS_ACCENT, true, 1);
		}

		// つまみ（外側はウィンドウ色のリング、内側がアクセント色）
		const int thumbX{ sliderLeft + filledWidth };
		const int thumbRadius{ scaled(SLIDER_THUMB_RADIUS) };
		m_uiRenderer.drawCircle(thumbX, centerY, thumbRadius, Color::SETTINGS_CARD, true, 1);
		m_uiRenderer.drawCircle(thumbX, centerY, thumbRadius, Color::SETTINGS_STROKE, false, 1);
		m_uiRenderer.drawCircle(thumbX, centerY, scaled(SLIDER_THUMB_INNER_RADIUS),
		    Color::SETTINGS_ACCENT, true, 1);
	}

	void SettingsPanelView::drawToggle(int right, int centerY, bool isOn) const
	{
		const int width{ scaled(TOGGLE_WIDTH) };
		const int height{ scaled(TOGGLE_HEIGHT) };
		const int left{ right - width };
		const int top{ centerY - height / 2 };
		const int radius{ height / 2 };

		if (isOn)
			m_uiRenderer.drawRoundedBox(left, top, width, height, radius, Color::SETTINGS_ACCENT, true, 1);
		else
		{
			m_uiRenderer.drawRoundedBox(left, top, width, height, radius, Color::SETTINGS_CARD, true, 1);
			m_uiRenderer.drawRoundedBox(left, top, width, height, radius, Color::SETTINGS_TEXT_SECONDARY, false, 1);
		}

		const int knobRadius{ scaled(TOGGLE_KNOB_RADIUS) };
		const int knobOffset{ scaled(10.0f) };
		const int knobX{ isOn ? right - knobOffset : left + knobOffset };
		m_uiRenderer.drawCircle(knobX, centerY, knobRadius,
		    isOn ? Color::BLACK : Color::SETTINGS_TEXT, true, 1);

		// オン／オフの文字はトグルの左に置く（Windows と同じ並び）
		const int fontSize{ scaled(FONT_ROW_TITLE) };
		const std::string label{ toDrawable(isOn ? "オン" : "オフ") };
		const int labelWidth{ m_uiRenderer.getTextWidth(label.c_str(), fontSize) };
		m_uiRenderer.drawText(left - scaled(12.0f) - labelWidth, centerY - fontSize / 2,
		    label.c_str(), Color::SETTINGS_TEXT_SECONDARY, fontSize);
	}

	void SettingsPanelView::drawResetButton(int right, int centerY) const
	{
		const int width{ scaled(BUTTON_WIDTH) };
		const int height{ scaled(BUTTON_HEIGHT) };
		const int left{ right - width };
		const int top{ centerY - height / 2 };
		const int radius{ scaled(CARD_RADIUS) };

		m_uiRenderer.drawRoundedBox(left, top, width, height, radius, Color::SETTINGS_CARD_HOVER, true, 1);
		m_uiRenderer.drawRoundedBox(left, top, width, height, radius, Color::SETTINGS_STROKE, false, 1);

		const int fontSize{ scaled(FONT_ROW_TITLE) };
		const std::string label{ toDrawable("リセット") };
		const int labelWidth{ m_uiRenderer.getTextWidth(label.c_str(), fontSize) };
		m_uiRenderer.drawText(left + (width - labelWidth) / 2, centerY - fontSize / 2,
		    label.c_str(), Color::SETTINGS_TEXT, fontSize);
	}

	std::string SettingsPanelView::toDrawable(const char* utf8) const
	{
		// DxLib のマルチバイト描画に合わせて UTF-8 から Shift-JIS へ変換する
		auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };
		if (!converter)
			return std::string{ utf8 };

		return converter->utf8ToShiftJis(utf8);
	}
} // namespace game::ui::settings
