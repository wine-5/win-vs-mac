#include "PadButtonIcon.h"
#include "core/constant/UI.h"
#include "core/utility/Color.h"
#include <algorithm>

namespace
{
	using game::ui::PadButton;

	// 記号の線の太さ。小さく描いても消えないよう、大きさに比例させる
	constexpr int THICKNESS_DIVISOR{ 9 };

	// L1・OPTIONS などの文字バッジ
	constexpr int BADGE_PADDING_X{ 6 };
	constexpr int BADGE_RADIUS{ 4 };
	constexpr int BADGE_FONT_DIVISOR{ 2 }; // バッジの高さに対する文字の大きさ

	// 記号を四角の中いっぱいに描くと窮屈なので、少し内側へ寄せる
	constexpr int INSET_NUMERATOR{ 1 };
	constexpr int INSET_DENOMINATOR{ 6 };

	/**
	 * @brief 文字バッジ（L1・OPTIONS など）に出す文字を返す
	 * @param button 対象のボタン
	 * @return 文字。図形で描くボタンなら nullptr
	 */
	const char* badgeText(PadButton button) noexcept
	{
		switch (button)
		{
		case PadButton::L1: return "L1";
		case PadButton::R1: return "R1";
		case PadButton::Options: return "OPTIONS";
		case PadButton::Share: return "SHARE";
		default: return nullptr;
		}
	}

	/**
	 * @brief 記号の色を返す
	 * @param button 対象のボタン
	 * @return 描画色
	 */
	unsigned int markColor(PadButton button) noexcept
	{
		switch (button)
		{
		case PadButton::Cross: return core::utility::Color::PAD_CROSS_BLUE;
		case PadButton::Circle: return core::utility::Color::PAD_CIRCLE_RED;
		case PadButton::Square: return core::utility::Color::PAD_SQUARE_PINK;
		case PadButton::Triangle: return core::utility::Color::PAD_TRIANGLE_GREEN;
		default: return core::utility::Color::HUD_INK;
		}
	}
} // namespace

namespace game::ui
{
	PadButtonIcon::PadButtonIcon(core::iface::IUIRenderer& uiRenderer) noexcept
	    : m_uiRenderer{ uiRenderer }
	{
	}

	int PadButtonIcon::measure(PadButton button, int size) const
	{
		const char* text{ badgeText(button) };
		if (text == nullptr)
			return size;

		const int fontSize{ std::max(1, size / BADGE_FONT_DIVISOR) };
		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
		const int textWidth{ m_uiRenderer.getTextWidth(text, fontSize) };
		m_uiRenderer.resetFont();

		return textWidth + BADGE_PADDING_X * 2;
	}

	int PadButtonIcon::draw(PadButton button, int x, int y, int size)
	{
		const int thickness{ std::max(1, size / THICKNESS_DIVISOR) };
		const unsigned int color{ markColor(button) };

		// L1 や OPTIONS は図形にしても伝わらないので、丸角の枠に文字を入れる
		if (const char* text{ badgeText(button) })
		{
			const int width{ measure(button, size) };
			const int fontSize{ std::max(1, size / BADGE_FONT_DIVISOR) };

			m_uiRenderer.drawRoundedBox(x, y, width, size, BADGE_RADIUS,
			    core::utility::Color::PAD_BADGE_BORDER, false, thickness);

			m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
			const int textWidth{ m_uiRenderer.getTextWidth(text, fontSize) };
			m_uiRenderer.drawText(x + (width - textWidth) / 2, y + (size - fontSize) / 2,
			    text, core::utility::Color::HUD_INK, fontSize);
			m_uiRenderer.resetFont();

			return width;
		}

		const int inset{ size * INSET_NUMERATOR / INSET_DENOMINATOR };
		const int left{ x + inset };
		const int top{ y + inset };
		const int right{ x + size - inset };
		const int bottom{ y + size - inset };
		const int centerX{ x + size / 2 };
		const int centerY{ y + size / 2 };

		switch (button)
		{
		case PadButton::Cross:
			m_uiRenderer.drawLine(left, top, right, bottom, color, thickness);
			m_uiRenderer.drawLine(right, top, left, bottom, color, thickness);
			break;

		case PadButton::Circle:
			m_uiRenderer.drawCircle(centerX, centerY, (right - left) / 2, color, false, thickness);
			break;

		case PadButton::Square:
			m_uiRenderer.drawRoundedBox(left, top, right - left, bottom - top,
			    thickness, color, false, thickness);
			break;

		case PadButton::Triangle:
			// 塗りではなく枠で描きたいが drawTriangle に太さが無いため、辺を線で引く
			m_uiRenderer.drawLine(centerX, top, right, bottom, color, thickness);
			m_uiRenderer.drawLine(right, bottom, left, bottom, color, thickness);
			m_uiRenderer.drawLine(left, bottom, centerX, top, color, thickness);
			break;

		case PadButton::DPad:
		{
			// 十字。中央の正方形を軸に、上下左右へ同じ太さの腕を伸ばす
			const int arm{ std::max(1, (right - left) / 3) };
			m_uiRenderer.drawBox(centerX - arm / 2, top, arm, bottom - top, color, true);
			m_uiRenderer.drawBox(left, centerY - arm / 2, right - left, arm, color, true);
			break;
		}

		case PadButton::LeftStick:
			// 外周がスティックの可動範囲、中の点が倒す先
			m_uiRenderer.drawCircle(centerX, centerY, (right - left) / 2, color, false, thickness);
			m_uiRenderer.drawCircle(centerX, centerY, std::max(1, (right - left) / 6), color, true, 1);
			break;

		default:
			break;
		}

		return size;
	}
} // namespace game::ui
