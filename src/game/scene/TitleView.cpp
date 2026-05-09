#include "TitleView.h"
#include "game/ui/Button.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"
#include <string>

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
		const int screenWidth { screen.getWidth() };
		const int screenHeight{ screen.getHeight() };
		const int buttonWidth { static_cast<int>(screenWidth  * BUTTON_WIDTH_RATIO) };
		const int buttonHeight{ static_cast<int>(screenHeight * BUTTON_HEIGHT_RATIO) };
		const int buttonX     { (screenWidth - buttonWidth) / 2 };
		const int startButtonY{ static_cast<int>(screenHeight * START_BUTTON_Y_RATIO) };
		const int exitButtonY { static_cast<int>(screenHeight * EXIT_BUTTON_Y_RATIO) };

		auto startBtn{ std::make_unique<ui::Button>(
			"選択画面へ", buttonX, startButtonY, buttonWidth, buttonHeight, inputProvider) };
		startBtn->setOnClick(std::move(onGoToSelect));
		startBtn->setVisible(false);
		m_startButton = startBtn.get();
		m_uiManager.addElement(std::move(startBtn));

		auto exitBtn{ std::make_unique<ui::Button>(
			"EXEを終了する", buttonX, exitButtonY, buttonWidth, buttonHeight, inputProvider) };
		exitBtn->setOnClick(std::move(onExit));
		exitBtn->setVisible(false);
		m_exitButton = exitBtn.get();
		m_uiManager.addElement(std::move(exitBtn));
	}

	void TitleView::update()
	{
		m_uiManager.update();
	}

	void TitleView::drawSplash(int dotCount) const
	{
		m_uiRenderer.setFont(m_mainFontName.c_str());

		std::string text{ "Win vs Mac.exeを起動しています" };
		for (int i{ 0 }; i < dotCount; ++i)
			text += '.';

		const int textWidth{ m_uiRenderer.getTextWidth(text.c_str(), core::constant::ui::DEFAULT_FONT_SIZE) };
		const int x{ (m_screen.getWidth() - textWidth) / 2 };
		const int y{ (m_screen.getHeight() - core::constant::ui::FONT_SIZE_NORMAL) / 2 };
		m_uiRenderer.drawText(x, y, text.c_str(), core::utility::Color::WHITE);
	}

	void TitleView::drawTitle() const
	{
		m_uiRenderer.setFont(m_mainFontName.c_str());

		const char* title{ "Win vs Mac" };
		const int titleWidth{ m_uiRenderer.getTextWidth(title, core::constant::ui::FONT_SIZE_LARGE) };
		const int titleX{ (m_screen.getWidth() - titleWidth) / 2 };
		const int titleY{ static_cast<int>(m_screen.getHeight() * TITLE_Y_RATIO) };

		m_uiRenderer.drawText(titleX, titleY, title, core::utility::Color::WHITE,
			core::constant::ui::FONT_SIZE_LARGE);
		m_uiManager.draw(m_uiRenderer);
	}

	void TitleView::setButtonsVisible(bool visible)
	{
		m_startButton->setVisible(visible);
		m_exitButton->setVisible(visible);
	}
}