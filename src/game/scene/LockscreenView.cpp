#include "LockscreenView.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"
#include <cmath>
#include <ctime>
#include <string>
#include <format>

namespace game::scene
{
	LockscreenView::LockscreenView(core::iface::IUIRenderer& uiRenderer,
		core::iface::IScreen& screen,
		std::string mainFontName)
		: m_uiRenderer{ uiRenderer }
		, m_screen{screen}
		, m_mainFontName{ std::move(mainFontName)}
	{
	}

	void LockscreenView::update(float deltaTime)
	{
		m_hintTimer += deltaTime;
	}

	void LockscreenView::draw(int offsetY) const
	{
		// 背景 TODO: もしかしたら画像に差し替える可能性がある
		m_uiRenderer.drawBox(0, offsetY, m_screen.getWidth(), m_screen.getHeight() + offsetY,
			BG_COLOR, true);

		//// 現在時刻の取得
		const std::time_t currentTime{ std::time(nullptr) };
		const std::tm* t{ std::localtime(&currentTime) };

		m_uiRenderer.setFont(m_mainFontName.c_str());

		// 時刻(HH:MMの部分）
		char timeStr[6]{};
		std::snprintf(timeStr, sizeof(timeStr), "%02d:%02d", t->tm_hour, t->tm_min);  // %02dは幅2桁で表示して足りない場合は0で埋める
		
		const int timeWidth{ m_uiRenderer.getTextWidth(timeStr) };
		const int timeX{ (m_screen.getWidth() - timeWidth) / 2 };
		const int timeY{ static_cast<int>(m_screen.getHeight() * TIME_Y_RATIO) + offsetY };
		m_uiRenderer.drawText(timeX, timeY, timeStr, core::utility::Color::WHITE,
			core::constant::ui::FONT_SIZE_LARGE);

		// 日付(M月D日(曜))
		const char* weekdays[]{ "日","月", "火", "水", "木", "金", "土" };
		const std::string dateStr{ std::format("{}月{}日({})",
			t->tm_mon + 1, t->tm_mday,weekdays[t->tm_wday])};

		const int dateWidth{ m_uiRenderer.getTextWidth(dateStr.c_str()) };
		const int dateX{ (m_screen.getWidth() - dateWidth) / 2 };
		const int dateY{ static_cast<int>(m_screen.getHeight() * DATE_Y_RATIO) + offsetY };
		m_uiRenderer.drawText(dateX, dateY, dateStr.c_str(), core::utility::Color::WHITE);

		// ヒント描画
		//const int 
		const char* hint{ "クリックまたきキーを押してください" };
		const int hintWidth{ m_uiRenderer.getTextWidth(hint) };
		const int hintX{ (m_screen.getWidth() - hintWidth) / 2 };
		const int hintY{ static_cast<int>(m_screen.getHeight() * HINT_Y_RATIO) + offsetY };
		m_uiRenderer.drawText(hintX, hintY, hint, core::utility::Color::WHITE);

		m_uiRenderer.resetBlendMode();
	}
};
