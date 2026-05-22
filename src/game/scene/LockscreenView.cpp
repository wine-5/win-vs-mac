#include "LockscreenView.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IStringConverter.h"
#include <cmath>
#include <ctime>
#include <string>

namespace game::scene
{
	LockscreenView::LockscreenView(core::iface::IUIRenderer& uiRenderer,
		core::iface::IScreen& screen,
		core::iface::IResourceManager& resourceManager,
		std::string mainFontName)
		: m_uiRenderer{ uiRenderer }
		, m_screen{screen}
		, m_mainFontName{ std::move(mainFontName)}
	{
		m_bgHandle = resourceManager.loadImageById("lock-bg");
	}

	void LockscreenView::update(float deltaTime)
	{
		m_hintTimer += deltaTime;
	}

	void LockscreenView::draw(int offsetY) const
	{
		// 背景を描画する
		if (m_bgHandle != -1)
			m_uiRenderer.drawImage(m_bgHandle, 0, offsetY, m_screen.getWidth(), m_screen.getHeight());
		else
			m_uiRenderer.drawBox(0, offsetY, m_screen.getWidth(), m_screen.getHeight() + offsetY,
				BG_COLOR, true);

		// 現在時刻の取得
		const std::time_t currentTime{ std::time(nullptr) };
		std::tm t{};
#ifdef _WIN32
		localtime_s(&t, &currentTime);
#else
		localtime_r(&currentTime, &t);
#endif

		m_uiRenderer.setFont(m_mainFontName.c_str());

		// 時刻(HH:MMの部分）
		char timeStr[6]{};
		std::snprintf(timeStr, sizeof(timeStr), "%02d:%02d", t.tm_hour, t.tm_min);  // %02dは幅2桁で表示して足りない場合は0で埋める

		const int clockFontSize{ static_cast<int>(m_screen.getHeight() * core::constant::ui::FONT_SIZE_CLOCK_RATIO) };
		const int timeWidth{ m_uiRenderer.getTextWidth(timeStr, clockFontSize) };
		const int timeX{ (m_screen.getWidth() - timeWidth) / 2 };
		const int timeY{ static_cast<int>(m_screen.getHeight() * TIME_Y_RATIO) + offsetY };
		m_uiRenderer.drawText(timeX, timeY, timeStr, core::utility::Color::WHITE, clockFontSize);

		// 日付(M月D日(曜))
		const char* weekdays[]{ "日", "月", "火", "水", "木", "金", "土" };
		char dateStr[32]{};
		std::snprintf(dateStr, sizeof(dateStr), "%d月%d日(%s)",
			t.tm_mon + 1, t.tm_mday, weekdays[t.tm_wday]);

		auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };
		std::string convertedDateStr{ dateStr };
		if (converter)
			convertedDateStr = converter->utf8ToShiftJis(convertedDateStr);

		const int normalFontSize{ static_cast<int>(m_screen.getHeight() * core::constant::ui::DEFAULT_FONT_SIZE_RATIO) };
		const int dateWidth{ m_uiRenderer.getTextWidth(convertedDateStr.c_str(), normalFontSize) };
		const int dateX{ (m_screen.getWidth() - dateWidth) / 2 };
		const int dateY{ static_cast<int>(m_screen.getHeight() * DATE_Y_RATIO) + offsetY };
		m_uiRenderer.drawText(dateX, dateY, convertedDateStr.c_str(), core::utility::Color::WHITE, normalFontSize);

		// ヒント描画
		const float sinVal{ std::sin(m_hintTimer * HINT_PULSE_SPEED) };
		const int alpha{ static_cast<int>((sinVal * HINT_ALPHA_RANGE + HINT_ALPHA_MIN) * 255) };
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);

		std::string hint{ "クリックまたはキーを押してください" };
		if (converter)
			hint = converter->utf8ToShiftJis(hint);

		const int hintFontSize{ static_cast<int>(m_screen.getHeight() * core::constant::ui::FONT_SIZE_EXTRA_SMALL_RATIO) };
		const int hintWidth{ m_uiRenderer.getTextWidth(hint.c_str(), hintFontSize) };
		const int hintX{ (m_screen.getWidth() - hintWidth) / 2 };
		const int hintY{ static_cast<int>(m_screen.getHeight() * HINT_Y_RATIO) + offsetY };
		m_uiRenderer.drawText(hintX, hintY, hint.c_str(), core::utility::Color::WHITE, hintFontSize);

		m_uiRenderer.resetBlendMode();
	}
}
