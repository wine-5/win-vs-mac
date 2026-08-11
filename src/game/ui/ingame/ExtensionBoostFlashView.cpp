#include "ExtensionBoostFlashView.h"
#include "core/base/ServiceLocator.h"
#include "core/constant/UI.h"
#include "core/interface/IStringConverter.h"
#include "core/utility/Color.h"
#include "game/utility/PlayerStats.h"
#include <algorithm>
#include <cstdio>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// 画面全体の閃光。長く残すと視界を塞ぐので、気付ける最短で消す
	constexpr float FLASH_DURATION{ 0.42f };
	constexpr int FLASH_ALPHA{ 150 };

	// 中央のメッセージ。閃光より長く残して、光ったあとに何が起きたのかを読ませる
	constexpr float MESSAGE_DURATION{ 2.6f };
	constexpr float MESSAGE_FADE_START{ 1.8f };    // ここから薄くしていく
	constexpr float MESSAGE_RISE_DURATION{ 0.3f }; // せり上がりにかける秒数
	constexpr int MESSAGE_RISE_DISTANCE{ 28 };     // せり上がる距離（1080p基準）

	// 倍率の数字。語より先に目へ入る大きさにする
	constexpr int MULTIPLIER_FONT_SIZE{ 92 };
	constexpr int MESSAGE_FONT_SIZE{ 28 };
	constexpr int MESSAGE_GAP{ 12 };       // 数字と説明文の間隔
	constexpr int MESSAGE_CENTER_Y{ 300 }; // 画面上端からの位置。中央だと自機と重なる

	// 文字の下敷き。3D空間の上へ直接置くと、背景の明るさ次第で白文字が溶ける。
	// 影や縁取りでも読めるようにはできるが、背景が何色でも同じ濃さで読めるのは下敷きだけ
	constexpr int PANEL_PADDING_X{ 40 };
	constexpr int PANEL_PADDING_Y{ 22 };
	constexpr int PANEL_RADIUS{ 8 };
	constexpr int PANEL_FILL_ALPHA{ 200 };
	constexpr int PANEL_BORDER_THICKNESS{ 2 };

	/**
	 * @brief UTF-8の文字列をDxLibが期待するShift_JISへ変換する
	 * @param text UTF-8の文字列
	 * @return 変換後の文字列（変換器が無ければそのまま）
	 */
	std::string toDrawable(const char* text)
	{
		auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };
		return converter ? converter->utf8ToShiftJis(text) : std::string{ text };
	}
} // namespace

namespace game::ui::ingame
{
	ExtensionBoostFlashView::ExtensionBoostFlashView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::ecs::ComponentManager& componentManager)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_componentManager{ componentManager }
	{
		m_message = toDrawable("拡張子の効果が上がった");
	}

	int ExtensionBoostFlashView::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	float ExtensionBoostFlashView::elapsedSeconds() const
	{
		return std::chrono::duration<float>(std::chrono::steady_clock::now() - m_startTime).count();
	}

	void ExtensionBoostFlashView::draw(core::ecs::EntityId playerId)
	{
		const float multiplier{ utility::playerBonusMultiplier(m_componentManager, playerId) };

		// 上がった瞬間だけ拾う。下がることは無いが、比較で書いておけば
		// 将来仕様が変わっても「上がったときだけ光る」が壊れない
		if (multiplier > m_previousMultiplier)
		{
			m_startTime = std::chrono::steady_clock::now();
			m_multiplier = multiplier;
			m_isPlaying = true;
		}
		m_previousMultiplier = multiplier;

		if (!m_isPlaying)
			return;

		const float elapsed{ elapsedSeconds() };
		if (elapsed >= MESSAGE_DURATION)
		{
			m_isPlaying = false;
			return;
		}

		drawFlash(elapsed);
		drawMessage(elapsed, m_multiplier);
	}

	void ExtensionBoostFlashView::drawFlash(float elapsed)
	{
		if (elapsed >= FLASH_DURATION)
			return;

		// 一気に明るくして滑らかに引く。線形で引くと消え際が唐突に見える
		const float remain{ 1.0f - elapsed / FLASH_DURATION };
		const int alpha{ static_cast<int>(FLASH_ALPHA * remain * remain) };
		if (alpha <= 0)
			return;

		// 加算合成にする。上から紫を被せると画面全体が濁って暗く見え、
		// 「光った」ではなく「フィルタが掛かった」になる
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ADD, alpha);
		m_uiRenderer.drawBox(0, 0, m_screen.getWidth(), m_screen.getHeight(),
		    core::utility::Color::HUD_EXTENSION_BOOST_VIOLET, true);
		m_uiRenderer.resetBlendMode();
	}

	void ExtensionBoostFlashView::drawMessage(float elapsed, float multiplier)
	{
		// 出たまま動かないと画面へ貼り付いた札に見える。短くせり上げて着地させる
		const float rise{ std::clamp(elapsed / MESSAGE_RISE_DURATION, 0.0f, 1.0f) };
		const int offsetY{ static_cast<int>(scaled(MESSAGE_RISE_DISTANCE) * (1.0f - rise)) };

		int alpha{ 255 };
		if (elapsed > MESSAGE_FADE_START)
		{
			const float fade{ (elapsed - MESSAGE_FADE_START) / (MESSAGE_DURATION - MESSAGE_FADE_START) };
			alpha = static_cast<int>(255 * (1.0f - fade));
		}
		if (alpha <= 0)
			return;

		char countText[32]{};
		std::snprintf(countText, sizeof(countText), "x%g", multiplier);

		const int centerX{ m_screen.getWidth() / 2 };
		const int countFontSize{ scaled(MULTIPLIER_FONT_SIZE) };
		const int messageFontSize{ scaled(MESSAGE_FONT_SIZE) };
		const int top{ scaled(MESSAGE_CENTER_Y) + offsetY };

		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
		const int countWidth{ m_uiRenderer.getTextWidth(countText, countFontSize) };
		const int messageWidth{ m_uiRenderer.getTextWidth(m_message.c_str(), messageFontSize) };

		// 下敷きは2行を囲める大きさにする。行ごとに敷くと段差が出て札が2枚に見える
		const int textWidth{ std::max(countWidth, messageWidth) };
		const int textHeight{ countFontSize + scaled(MESSAGE_GAP) + messageFontSize };
		const int panelX{ centerX - textWidth / 2 - scaled(PANEL_PADDING_X) };
		const int panelY{ top - scaled(PANEL_PADDING_Y) };
		const int panelWidth{ textWidth + scaled(PANEL_PADDING_X) * 2 };
		const int panelHeight{ textHeight + scaled(PANEL_PADDING_Y) * 2 };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA,
		    PANEL_FILL_ALPHA * alpha / 255);
		m_uiRenderer.drawRoundedBox(panelX, panelY, panelWidth, panelHeight,
		    scaled(PANEL_RADIUS), core::utility::Color::HUD_PANEL_FILL, true, 1);
		m_uiRenderer.resetBlendMode();

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);
		m_uiRenderer.drawRoundedBox(panelX, panelY, panelWidth, panelHeight,
		    scaled(PANEL_RADIUS), core::utility::Color::HUD_EXTENSION_BOOST_VIOLET, false,
		    scaled(PANEL_BORDER_THICKNESS));

		m_uiRenderer.drawText(centerX - countWidth / 2, top, countText,
		    core::utility::Color::HUD_EXTENSION_BOOST_VIOLET, countFontSize);
		m_uiRenderer.drawText(centerX - messageWidth / 2,
		    top + countFontSize + scaled(MESSAGE_GAP), m_message.c_str(),
		    core::utility::Color::HUD_INK, messageFontSize);
		m_uiRenderer.resetFont();

		m_uiRenderer.resetBlendMode();
	}
} // namespace game::ui::ingame
