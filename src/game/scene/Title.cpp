#include "Title.h"
#include "TitleView.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IResourceManager.h"
#include <cstdlib>
#include "core/interface/IPerformanceDataProvider.h"
#include "core/interface/IAudioManager.h"
#include "core/constant/BgmType.h"

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

		// Loading 画面で起動演出済みのため、タイトルはフェードインから開始する
		m_view->setButtonsVisible(true);
		m_fade = std::make_unique<ui::FadeTransition>(
			m_uiRenderer, m_screen, FADE_DURATION, true);

		m_perfProvider = core::base::ServiceLocator::get<core::iface::IPerformanceDataProvider>();

		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio) audio->playBgm(core::constant::BgmType::Title);
	}

	Title::~Title() noexcept = default;

	void Title::update(float deltaTime)
	{
		m_perfTimer += deltaTime;
		if (m_perfTimer >= PERF_UPDATE_INTERVAL)
		{
			m_perfTimer -= PERF_UPDATE_INTERVAL;
			m_perfProvider->update();
		}

		switch (m_state)
		{
		case State::TitleFadeIn:
		{
			const auto snap{ m_perfProvider->getSnapshot() };
			m_fade->update(deltaTime);
			m_view->update(snap);
			if (m_fade->isFinished())
			{
				m_fade = nullptr;
				m_state = State::Idle;
			}
			break;
		}

		case State::Idle:
		{
			const auto snap{ m_perfProvider->getSnapshot() };
			m_view->update(snap);
			break;
		}

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
		m_view->drawTitle();
		if (m_fade)
			m_fade->draw(m_uiRenderer, m_screen);
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
} // namespace game::scene
