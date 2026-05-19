#include "Title.h"
#include "TitleView.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IResourceManager.h"
#include <cstdlib>

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

		m_view = std::make_unique<TitleView>(
			inputProvider, uiRenderer, screen,
			std::move(mainFontName),
			[this]() { goToSelect(); },
			[this]() { exitApp(); });

		// Loading 画面で起動演出済みのため、Splash をスキップしてタイトルをフェードインで表示
		m_view->setButtonsVisible(true);
		m_fade = std::make_unique<ui::FadeTransition>(
			m_uiRenderer, m_screen, FADE_DURATION, true);
	}

	void Title::update(float deltaTime)
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
				auto* sceneManager{ core::base::ServiceLocator::get<game::scene::SceneManager>() };
				sceneManager->changeScene(SceneType::Select);
			}
			break;
		}
	}

	void Title::draw()
	{
		switch (m_state)
		{	
		case State::Splash:
		case State::SplashFadeOut:
			m_view->drawSplash(m_dotCount);
			if (m_fade) m_fade->draw(m_uiRenderer, m_screen);
			break;

		case State::TitleFadeIn:
		case State::Idle:
		case State::FadingOut:
			m_view->drawTitle();
			if (m_fade) m_fade->draw(m_uiRenderer, m_screen);
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
