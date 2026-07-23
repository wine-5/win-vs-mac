#include "Select.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "core/interface/IAudioManager.h"
#include "core/constant/BgmType.h"

namespace game::scene
{
	Select::Select(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::iface::IResourceManager& resourceManager,
	    std::unique_ptr<core::iface::ISelectWindowManager> windowManager)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_resourceManager{ resourceManager }
	    , m_windowManager{ std::move(windowManager) }
	    , m_fade{ std::make_unique<ui::FadeTransition>(uiRenderer, screen, FADE_DURATION, true) }
	{
		if (m_windowManager)
			m_windowManager->createAllWindows();

		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio) audio->playBgm(core::constant::BgmType::Select);
	}

	Select::~Select() noexcept
	{
		m_fade.reset();
		if (m_windowManager)
			m_windowManager->destroyAllWindows();
	}

	void Select::update(float deltaTime)
	{
		if (m_fade)
			m_fade->update(deltaTime);

		switch (m_state)
		{
		case State::FadeIn:
			if (m_fade && m_fade->isFinished())
			{
				m_fade.reset();
				m_state = State::Idle;
			}
			break;

		case State::Idle:
			if (m_windowManager)
				m_windowManager->pumpMessages();
			break;

		case State::FadeOut:
			if (m_fade && m_fade->isFinished())
			{
				auto* sceneManager{ core::base::ServiceLocator::get<SceneManager>() };
				sceneManager->changeScene(SceneType::Loading);
			}
			break;
		}
	}

	void Select::draw()
	{
		if (m_fade)
			m_fade->draw(m_uiRenderer, m_screen);
	}

	void Select::startFadeOut()
	{
		if (m_state == State::FadeOut)
		{
			return;
		}
		if (m_windowManager)
			m_windowManager->destroyAllWindows();
		m_fade = std::make_unique<ui::FadeTransition>(m_uiRenderer, m_screen, FADE_DURATION, false);
		m_state = State::FadeOut;
	}

	void Select::setWindowManager(std::unique_ptr<core::iface::ISelectWindowManager> windowManager) noexcept
	{
		m_windowManager = std::move(windowManager);
		if (m_windowManager)
			m_windowManager->createAllWindows();
	}

	void Select::notifyGameStart() noexcept
	{
		if (!m_windowManager)
			return;
		startFadeOut();
	}

} // namespace game::scene
