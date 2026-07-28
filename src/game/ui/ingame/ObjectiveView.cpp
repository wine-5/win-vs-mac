#include "ObjectiveView.h"
#include "core/base/ServiceLocator.h"
#include "core/constant/UI.h"
#include "core/interface/IStringConverter.h"
#include "core/utility/Color.h"
#include "core/utility/MathConstants.h"
#include <algorithm>
#include <cmath>
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

	constexpr const char* MONO_FONT_NAME{ "Cascadia Mono SemiBold" };
	constexpr const char* UI_FONT_NAME{ "Noto Sans JP" };

	// 残り数が減ったときの反応
	constexpr float COUNT_REACTION_DURATION{ 0.45f };
	constexpr int COUNT_REACTION_POP{ 7 }; // 跳ね上がる最大量（1080p基準・上方向）

	constexpr const char* CAPTION_TEXT{ "目標" };
	constexpr const char* DETAIL_TEXT{ "体 倒すと Mac が出現" };
	constexpr const char* BOSS_TEXT{ "Mac を破壊せよ" };

	/**
	 * @brief 2色を線形補間する
	 * @param from 進行度0.0のときの色（ARGB形式：0xAARRGGBB）
	 * @param to 進行度1.0のときの色（ARGB形式：0xAARRGGBB）
	 * @param t 進行度（0.0〜1.0）
	 * @return 補間した色（アルファは不透明で返す）
	 */
	unsigned int lerpColor(unsigned int from, unsigned int to, float t)
	{
		auto channel = [](unsigned int color, int shift)
		{ return static_cast<int>((color >> shift) & 0xFFu); };
		auto blend = [&](int shift)
		{
			const int a{ channel(from, shift) };
			const int b{ channel(to, shift) };
			return a + static_cast<int>((b - a) * t);
		};
		return core::utility::Color::argb(255, blend(16), blend(8), blend(0));
	}
} // namespace

namespace game::ui::ingame
{
	ObjectiveView::ObjectiveView(core::iface::IUIRenderer& uiRenderer, core::iface::IScreen& screen)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_panel{ uiRenderer, screen }
	    , m_captionText{ CAPTION_TEXT }
	    , m_detailText{ DETAIL_TEXT }
	    , m_bossText{ BOSS_TEXT }
	{
		// DxLibの描画はShift_JISを期待する。ソース上のUTF-8をそのまま渡すと日本語が化けるため、
		// ここで一度だけ変換しておく（変換結果は毎フレーム同じなので描画時にはやらない）
		auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };
		if (!converter)
			return;

		m_captionText = converter->utf8ToShiftJis(m_captionText);
		m_detailText = converter->utf8ToShiftJis(m_detailText);
		m_bossText = converter->utf8ToShiftJis(m_bossText);
	}

	int ObjectiveView::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	void ObjectiveView::updateCountReaction(int remainingEnemyCount)
	{
		if (remainingEnemyCount == m_lastCount)
			return;

		// 初回（-1からの初期化）とボス召喚などで増えた場合は反応させない。
		// 「1体倒した」という手応えを返すのが目的なので、減ったときだけ動かす
		const bool decreased{ m_lastCount >= 0 && remainingEnemyCount < m_lastCount };
		m_lastCount = remainingEnemyCount;
		if (!decreased)
			return;

		m_countChangedTime = std::chrono::steady_clock::now();
		m_isCountReacting = true;
	}

	float ObjectiveView::getCountReactionProgress() const
	{
		if (!m_isCountReacting)
			return 1.0f;

		const float elapsed{ std::chrono::duration<float>(
			std::chrono::steady_clock::now() - m_countChangedTime)
			    .count() };
		return std::clamp(elapsed / COUNT_REACTION_DURATION, 0.0f, 1.0f);
	}

	void ObjectiveView::draw(int remainingEnemyCount, bool isBossAppeared)
	{
		const int panelX{ scaled(PANEL_MARGIN) };
		const int panelY{ scaled(PANEL_MARGIN) };
		const int panelWidth{ scaled(PANEL_WIDTH) };
		const int panelHeight{ scaled(PANEL_HEIGHT) };

		updateCountReaction(remainingEnemyCount);

		m_panel.draw(panelX, panelY, panelWidth, panelHeight);

		const int padding{ scaled(PANEL_PADDING) };

		m_uiRenderer.setFont(UI_FONT_NAME);
		m_uiRenderer.drawText(panelX + padding, panelY + scaled(CAPTION_Y), m_captionText.c_str(),
		    core::utility::Color::HUD_INK, scaled(CAPTION_FONT_SIZE));

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

		// 減った瞬間だけ黄色く光らせて跳ねさせる。数字が変わったこと自体に気づけるようにする。
		// 文字サイズは変えない（サイズごとにフォントハンドルが増えるため）
		const float reaction{ getCountReactionProgress() };
		const unsigned int countColor{ lerpColor(core::utility::Color::HUD_CHARGE_MAX,
			core::utility::Color::HUD_INK, reaction) };
		// sinで上へ跳ねて戻る。開始と終了がどちらも0になるので継ぎ目が出ない
		const int popOffset{ static_cast<int>(
			-scaled(COUNT_REACTION_POP) * std::sin(reaction * core::utility::PI)) };

		m_uiRenderer.setFont(MONO_FONT_NAME);
		const int countFontSize{ scaled(COUNT_FONT_SIZE) };
		m_uiRenderer.drawText(panelX + padding, panelY + scaled(COUNT_Y) + popOffset, countText,
		    countColor, countFontSize);
		const int countWidth{ m_uiRenderer.getTextWidth(countText, countFontSize) };
		m_uiRenderer.resetFont();

		m_uiRenderer.setFont(UI_FONT_NAME);
		m_uiRenderer.drawText(panelX + padding + countWidth + scaled(COUNT_DETAIL_GAP),
		    panelY + scaled(DETAIL_Y), m_detailText.c_str(),
		    core::utility::Color::HUD_INK, scaled(DETAIL_FONT_SIZE));
		m_uiRenderer.resetFont();
	}

} // namespace game::ui::ingame
