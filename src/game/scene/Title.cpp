#include "Title.h"
#include "TitleView.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IResourceManager.h"
#include <cstdlib>
#include "LockscreenView.h"

namespace game::scene
{
	Title::Title(core::iface::IInputProvider& inputProvider,
		core::iface::IUIRenderer& uiRenderer,
		core::iface::IScreen& screen)
		: m_inputProvider{ inputProvider }
		, m_uiRenderer{ uiRenderer }
		, m_screen{ screen }
	{
		auto* res{ core::base::ServiceLocator::get<core::iface::IResourceManager>() };
		std::string mainFontName{ res->getFontName("main").value_or("") };
		std::string lockFontName{ res->getFontName("normal").value_or("") };
		m_lockscreenView = std::make_unique<LockscreenView>(uiRenderer, screen, std::move(lockFontName));

		m_view = std::make_unique<TitleView>(
			inputProvider, uiRenderer, screen,
			std::move(mainFontName),
			[this]() { goToSelect(); },
			[this]() { exitApp(); });
	}

	void Title::update(float deltaTime)
	{
		switch (m_state)
		{
		case State::Lockscreen:
			m_lockscreenView->update(deltaTime);
			if (m_inputProvider.isKeyPressed(core::input::KeyCode::Space)
				|| m_inputProvider.isMouseLeftPressed()
				|| m_inputProvider.isMouseRightPressed())
			{
				m_state = State::LockscreenSliding;
			}
			break;

		case State::LockscreenSliding:
		{
			m_lockscreenView->update(deltaTime);
			const float slideSpeed{ static_cast<float>(m_screen.getHeight()) / LOCKSCREEN_SLIDE_DURATION };
			m_lockscreenOffsetY -= slideSpeed * deltaTime;
			if (m_lockscreenOffsetY <= -static_cast<float>(m_screen.getHeight()))
				m_state = State::Splash;
			break;
		}

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
				m_view->setButtonsVisible(true);
				m_fade = std::make_unique<ui::FadeTransition>(
					m_uiRenderer, m_screen, FADE_DURATION, true);
				m_state = State::TitleFadeIn;
			}
			break;

		case State::TitleFadeIn:
			m_fade->update(deltaTime);
			m_view->update();
			if (m_fade->isFinished())
			{
				m_fade = nullptr;
				m_state = State::Idle;
			}
			break;

		case State::Idle:
			m_view->update();
			break;

		case State::FadingOut:
			m_fade->update(deltaTime);
			if (m_fade->isFinished())
			{
				auto* sceneManager{ core::base::ServiceLocator::get<SceneManager>() };
				sceneManager->changeScene(SceneType::Select);
			}
			break;
		}
	}

	void Title::draw()
	{
		switch (m_state)
		{	
		case State::Lockscreen:
		case State::LockscreenSliding:
			m_lockscreenView->draw(static_cast<int>(m_lockscreenOffsetY));
			break;

		case State::Splash:
		case State::SplashFadeOut:
			m_view->drawSplash(m_dotCount);
			if (m_fade) m_fade->draw();
			break;

		case State::TitleFadeIn:
		case State::Idle:
		case State::FadingOut:
			m_view->drawTitle();
			if (m_fade) m_fade->draw();
			break;
		}
	}

	void Title::goToSelect()
	{
		if (m_state != State::Idle) return;
		m_fade = std::make_unique<ui::FadeTransition>(
			m_uiRenderer, m_screen, FADE_DURATION, false);
		m_state = State::FadingOut;
	}

	void Title::exitApp()
	{
		if (m_state != State::Idle) return;
		std::exit(0);
	}
}
