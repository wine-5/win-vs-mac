#include "TitleView.h"
#include "game/ui/Button.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IStringConverter.h"
#include <string>
#include <algorithm>

namespace game::scene
{
	TitleView::TitleView(core::iface::IInputProvider& inputProvider,
		core::iface::IUIRenderer& uiRenderer,
		core::iface::IScreen& screen,
		std::string mainFontName,
		std::function<void()> onGoToSelect,
		std::function<void()> onExit)
		: m_uiRenderer{ uiRenderer }
		, m_screen{ screen }
		, m_mainFontName{ std::move(mainFontName) }
	{
		const int screenWidth    { screen.getWidth() };
		const int screenHeight   { screen.getHeight() };
		const int buttonWidth    { static_cast<int>(screenWidth  * BUTTON_WIDTH_RATIO) };
		const int buttonHeight   { static_cast<int>(screenHeight * BUTTON_HEIGHT_RATIO) };
		const int buttonX        { (screenWidth - buttonWidth) / 2 };
		const int startButtonY   { static_cast<int>(screenHeight * START_BUTTON_Y_RATIO) };
		const int exitButtonY    { static_cast<int>(screenHeight * EXIT_BUTTON_Y_RATIO) };
		const int buttonFontSize { static_cast<int>(screenHeight * core::constant::ui::DEFAULT_FONT_SIZE_RATIO) };

		auto startBtn{ std::make_unique<ui::Button>(
			"選択画面へ", buttonX, startButtonY, buttonWidth, buttonHeight, inputProvider, buttonFontSize) };
		startBtn->setOnClick(std::move(onGoToSelect));
		startBtn->setVisible(false);
		m_startButton = startBtn.get();
		m_uiManager.addElement(std::move(startBtn));

		auto exitBtn{ std::make_unique<ui::Button>(
			"EXEを終了する", buttonX, exitButtonY, buttonWidth, buttonHeight, inputProvider, buttonFontSize) };
		exitBtn->setOnClick(std::move(onExit));
		exitBtn->setVisible(false);
		m_exitButton = exitBtn.get();
		m_uiManager.addElement(std::move(exitBtn));
	}

	void TitleView::update(const core::iface::PerformanceSnapshot& snap)
	{
		pushHistory(m_cpuHistory,  snap.cpuUsage);
		pushHistory(m_memHistory,  snap.memoryUsage);
		pushHistory(m_diskHistory, snap.diskActivity);

		m_uiManager.update();
	}

	void TitleView::drawSplash(int dotCount) const
	{
		m_uiRenderer.setFont(m_mainFontName.c_str());

		std::string text{ "Win vs Mac.exeを起動しています" };
		for (int i{ 0 }; i < dotCount; ++i)
			text += '.';

		auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };
		if (converter)
			text = converter->utf8ToShiftJis(text);

		const int normalFontSize{ static_cast<int>(m_screen.getHeight() * core::constant::ui::DEFAULT_FONT_SIZE_RATIO) };
		const int textWidth{ m_uiRenderer.getTextWidth(text.c_str(), normalFontSize) };
		const int x{ (m_screen.getWidth() - textWidth) / 2 };
		const int y{ (m_screen.getHeight() - normalFontSize) / 2 };
		m_uiRenderer.drawText(x, y, text.c_str(), core::utility::Color::WHITE, normalFontSize);
	}

	void TitleView::drawTitle() const
	{
		drawBackground();

		m_uiRenderer.setFont(m_mainFontName.c_str());

		const char* title{ "Win vs Mac" };
		const int titleFontSize{ static_cast<int>(m_screen.getHeight() * core::constant::ui::FONT_SIZE_CLOCK_RATIO) };
		const int titleWidth{ m_uiRenderer.getTextWidth(title, titleFontSize) };
		const int titleX{ (m_screen.getWidth() - titleWidth) / 2 };
		const int titleY{ static_cast<int>(m_screen.getHeight() * TITLE_Y_RATIO) };

		m_uiRenderer.drawText(titleX, titleY, title, core::utility::Color::WHITE, titleFontSize);
		m_uiManager.draw(m_uiRenderer);
	}

	void TitleView::setButtonsVisible(bool visible)
	{
		m_startButton->setVisible(visible);
		m_exitButton->setVisible(visible);
	}

	void TitleView::pushHistory(std::array<float, HISTORY_SIZE>& buf, float value)
	{
		std::rotate(buf.begin(), buf.begin() + 1, buf.end());
		buf.back() = value;
	}

	void TitleView::drawBackground() const
	{
		const int W{ m_screen.getWidth()  };
		const int H{ m_screen.getHeight() };

		struct Channel
		{
			const std::array<float, HISTORY_SIZE>& history;
			const char*  label;
			unsigned int color;
			float        yRatio;
			float        heightRatio;
		};

		const Channel channels[]
		{
			{ m_cpuHistory,  "CPU",    core::utility::Color::rgb( 64, 128, 255), 0.25f, 0.10f },
			{ m_memHistory,  "Memory", core::utility::Color::rgb( 64, 255, 128), 0.50f, 0.10f },
			{ m_diskHistory, "Disk",   core::utility::Color::rgb(128, 255, 255), 0.75f, 0.10f },
		};

		const int graphW  { static_cast<int>(W * 0.85f) };
		const int graphX  { (W - graphW) / 2 };
		const int labelSize{ static_cast<int>(H * 0.022f) };
		const int barW    { graphW / HISTORY_SIZE };

		for (const auto& ch : channels)
		{
			const int centerY  { static_cast<int>(H * ch.yRatio)      };
			const int amplitude{ static_cast<int>(H * ch.heightRatio) };

			// ラベル
			m_uiRenderer.setBlendMode(2, 140);
			m_uiRenderer.drawText(graphX, centerY - amplitude - labelSize - 2,
				ch.label, ch.color, labelSize);

			// 波形
			for (int i{ 0 }; i < HISTORY_SIZE; ++i)
			{
				const int barH{ static_cast<int>(ch.history[i] * amplitude) };
				const int x   { graphX + i * barW };
				const int y   { centerY - barH };

				// 塗り（半透明）
				m_uiRenderer.setBlendMode(2, 45);
				m_uiRenderer.drawBox(x, y, barW, barH * 2, ch.color, true);

				// 最新値を示すカーソル
				if (i == HISTORY_SIZE - 1)
				{
					m_uiRenderer.setBlendMode(2, 200);
					m_uiRenderer.drawBox(x, y, 2, 4, ch.color, true);
				}
			}
			m_uiRenderer.resetBlendMode();
		}
	}
}