#include "TitleView.h"
#include "game/ui/Button.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"

namespace game::scene
{
	TitleView::TitleView(core::iface::IInputProvider& inputProvider,
		core::iface::IUIRenderer& uiRenderer,
		core::iface::IScreen& screen)
		: m_inputProvider{ inputProvider }
		, m_uiRenderer{ uiRenderer }
		, m_screen{ screen }
	{
		setupUI();
	}

	void TitleView::update(float deltaTime)
	{
		switch (m_state)
		{
		case State::Splash:
			m_splashTimer += deltaTime;
			m_dotTimer += deltaTime;
			if (m_dotTimer >= DOT_INTERVAL)
			{
				m_dotTimer -= DOT_INTERVAL;
				m_dotCount = (m_dotCount + 1) % (MAX_DOTS + 1);
			}
			if (m_splashTimer >= SPLASH_DURATION)
			{
				m_fade = std::make_unique<ui::FadeTransition>(
					m_uiRenderer, m_screen, FADE_DURATION, false);
				m_state = State::SplashFadeOut;
			}
			break;

		case State::SplashFadeOut:
			m_fade->update(deltaTime);
			if (m_fade->isFinished())
			{
				m_startButton->setVisible(true);
				m_exitButton->setVisible(true);
				m_fade = std::make_unique<ui::FadeTransition>(
					m_uiRenderer, m_screen, FADE_DURATION, true);
				m_state = State::TitleFadeIn;
			}
			break;

		case State::TitleFadeIn:
			m_fade->update(deltaTime);
			m_uiManager.update();
			if (m_fade->isFinished())
			{
				m_fade = nullptr;
				m_state = State::Idle;
			}
			break;

		case State::Idle:
			m_uiManager.update();
			break;

		case State::FadingOut:
			m_fade->update(deltaTime);
			break;
		}
	}

	void TitleView::draw()
	{
		switch (m_state)
		{
		case State::Splash:
		case State::SplashFadeOut:
		{
			const std::string text{ getSplashText() };
			const int textWidth{ m_uiRenderer.getTextWidth(text.c_str()) };
			const int x{ (m_screen.getWidth() - textWidth) / 2 };
			const int y{ (m_screen.getHeight() - core::constant::ui::FONT_SIZE_NORMAL) / 2 };
			m_uiRenderer.drawText(x, y, text.c_str(), core::utility::Color::WHITE);
			if (m_fade)
				m_fade->draw();
			break;
		}

		case State::TitleFadeIn:
		case State::Idle:
		case State::FadingOut:
		{
			const char* title{ "Win vs Mac" };
			const int titleWidth{ m_uiRenderer.getTextWidth(title) };
			const int titleX{ (m_screen.getWidth() - titleWidth) / 2 };
			const int titleY{ static_cast<int>(m_screen.getHeight() * TITLE_Y_RATIO) };
			m_uiRenderer.drawText(titleX, titleY, title, core::utility::Color::WHITE,
				core::constant::ui::FONT_SIZE_LARGE);
			m_uiManager.draw(m_uiRenderer);
			if (m_fade)
				m_fade->draw();
			break;
		}
		}
	}

	bool TitleView::isReadyToChange() const
	{
		return m_state == State::FadingOut && m_fade && m_fade->isFinished();
	}

	TitleView::Action TitleView::getNextAction() const
	{
		return m_nextAction;
	}

	void TitleView::setupUI()
	{
		const int screenWidth{ m_screen.getWidth() };
		const int screenHeight{ m_screen.getHeight() };
		const int buttonWidth{ static_cast<int>(screenWidth * BUTTON_WIDTH_RATIO) };
		const int buttonHeight{ static_cast<int>(screenHeight * BUTTON_HEIGHT_RATIO) };
		const int buttonX{ (screenWidth - buttonWidth) / 2 };
		const int startButtonY{ static_cast<int>(screenHeight * START_BUTTON_Y_RATIO) };
		const int exitButtonY{ static_cast<int>(screenHeight * EXIT_BUTTON_Y_RATIO) };

		auto startBtn{ std::make_unique<ui::Button>(
			"選択画面へ", buttonX, startButtonY, buttonWidth, buttonHeight, m_inputProvider) };
		startBtn->setOnClick([this]() { startFadeOut(Action::GoToStageSelect); });
		startBtn->setVisible(false);
		m_startButton = startBtn.get();
		m_uiManager.addElement(std::move(startBtn));

		auto exitBtn{ std::make_unique<ui::Button>(
			"EXEを終了する", buttonX, exitButtonY, buttonWidth, buttonHeight, m_inputProvider) };
		exitBtn->setOnClick([this]() { startFadeOut(Action::Exit); });
		exitBtn->setVisible(false);
		m_exitButton = exitBtn.get();
		m_uiManager.addElement(std::move(exitBtn));
	}

	void TitleView::startFadeOut(Action action)
	{
		if (m_state != State::Idle) return;
		m_nextAction = action;
		m_fade = std::make_unique<ui::FadeTransition>(
			m_uiRenderer, m_screen, FADE_DURATION, false);
		m_state = State::FadingOut;
	}

	std::string TitleView::getSplashText() const
	{
		std::string text{ "Win vs Mac.exeを起動しています" };
		for (int i{ 0 }; i < m_dotCount; ++i)
			text += '.';
		return text;
	}
}