#include "PadConnectedToast.h"
#include "core/base/ServiceLocator.h"
#include "core/constant/UI.h"
#include "core/interface/IStringConverter.h"
#include "core/utility/Color.h"
#include "core/utility/Easing.h"
#include <algorithm>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// 画面の右下からの余白
	constexpr int MARGIN_X{ 32 };
	constexpr int MARGIN_Y{ 32 };

	// カードの中身
	constexpr int PADDING_X{ 20 };
	constexpr int PADDING_Y{ 16 };
	constexpr int RADIUS{ 8 };
	constexpr int ICON_SIZE{ 26 };
	constexpr int ICON_GAP{ 14 };
	constexpr int FONT_SIZE{ 21 };

	// 滑り込む距離。長いと視線が追いつかず、短いと出たことに気付けない
	constexpr int SLIDE_DISTANCE{ 46 };

	// 面と枠。タイトル画面が明るいので、Windows 11 の通知と同じ白いカードにする
	constexpr unsigned int FILL_COLOR{ core::utility::Color::TITLE_CARD };
	constexpr unsigned int BORDER_COLOR{ core::utility::Color::TITLE_STROKE };
	constexpr unsigned int TEXT_COLOR{ core::utility::Color::TITLE_TEXT };

	// 左端に立てるアクセントの帯。通知の種類を色で示す Windows の作法に合わせる
	constexpr int ACCENT_BAR_WIDTH{ 4 };
	constexpr unsigned int ACCENT_COLOR{ core::utility::Color::TITLE_ACCENT };
} // namespace

namespace game::ui
{
	PadConnectedToast::PadConnectedToast(core::iface::IInputProvider& inputProvider,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen)
	    : m_inputProvider{ inputProvider }
	    , m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_padButtonIcon{ uiRenderer }
	{
		auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };
		const char* text{ "コントローラーが接続されています" };
		m_message = converter ? converter->utf8ToShiftJis(text) : text;
	}

	int PadConnectedToast::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	float PadConnectedToast::visibility() const
	{
		switch (m_phase)
		{
		case Phase::SlideIn: return core::utility::easeOut(m_phaseTime / SLIDE_DURATION);
		case Phase::Hold: return 1.0f;
		case Phase::SlideOut: return 1.0f - core::utility::easeOut(m_phaseTime / SLIDE_DURATION);
		default: return 0.0f;
		}
	}

	void PadConnectedToast::update(float deltaTime)
	{
		// 繋がった瞬間だけ出す。繋がっている間ずっと出し続けると、
		// 読み終えたあとも画面の隅を占め続けることになる
		const bool isConnected{ m_inputProvider.isPadConnected() };
		if (isConnected && !m_wasConnected)
		{
			m_phase = Phase::SlideIn;
			m_phaseTime = 0.0f;
		}
		m_wasConnected = isConnected;

		if (m_phase == Phase::Hidden)
			return;

		m_phaseTime += deltaTime;

		switch (m_phase)
		{
		case Phase::SlideIn:
			if (m_phaseTime < SLIDE_DURATION)
				return;

			m_phase = Phase::Hold;
			m_phaseTime = 0.0f;
			return;

		case Phase::Hold:
			if (m_phaseTime < HOLD_DURATION)
				return;

			m_phase = Phase::SlideOut;
			m_phaseTime = 0.0f;
			return;

		case Phase::SlideOut:
			if (m_phaseTime < SLIDE_DURATION)
				return;

			m_phase = Phase::Hidden;
			m_phaseTime = 0.0f;
			return;

		default:
			return;
		}
	}

	void PadConnectedToast::draw()
	{
		const float rate{ visibility() };
		if (rate <= 0.0f)
			return;

		const int fontSize{ scaled(FONT_SIZE) };
		const int iconSize{ scaled(ICON_SIZE) };

		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
		const int textWidth{ m_uiRenderer.getTextWidth(m_message.c_str(), fontSize) };
		m_uiRenderer.resetFont();

		const int contentWidth{ iconSize + scaled(ICON_GAP) + textWidth };
		const int cardWidth{ contentWidth + scaled(PADDING_X) * 2 };
		const int cardHeight{ std::max(iconSize, fontSize) + scaled(PADDING_Y) * 2 };

		// 出し切ったときの位置を右下に置き、そこから左へずらした位置から寄せてくる
		const int restX{ m_screen.getWidth() - scaled(MARGIN_X) - cardWidth };
		const int offset{ static_cast<int>(scaled(SLIDE_DISTANCE) * (1.0f - rate)) };
		const int left{ restX - offset };
		const int top{ m_screen.getHeight() - scaled(MARGIN_Y) - cardHeight };

		const int alpha{ static_cast<int>(255 * rate) };
		const int radius{ scaled(RADIUS) };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);

		m_uiRenderer.drawRoundedBox(left, top, cardWidth, cardHeight, radius, FILL_COLOR, true, 1);
		m_uiRenderer.drawRoundedBox(left, top, cardWidth, cardHeight, radius, BORDER_COLOR, false, 1);

		// 左端のアクセント。角丸の内側へ収めるため、上下を少し詰める
		const int barInset{ radius / 2 };
		m_uiRenderer.drawBox(left, top + barInset, scaled(ACCENT_BAR_WIDTH),
		    cardHeight - barInset * 2, ACCENT_COLOR, true);

		const int iconX{ left + scaled(PADDING_X) };
		const int iconY{ top + (cardHeight - iconSize) / 2 };

		// 〇を1つ描くだけで「パッドの話だ」と伝わる。機種ごとの絵を持たずに済む
		m_padButtonIcon.draw(PadButton::Circle, iconX, iconY, iconSize);

		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
		m_uiRenderer.drawText(iconX + iconSize + scaled(ICON_GAP),
		    top + (cardHeight - fontSize) / 2, m_message.c_str(), TEXT_COLOR, fontSize);
		m_uiRenderer.resetFont();

		m_uiRenderer.resetBlendMode();
	}
} // namespace game::ui
