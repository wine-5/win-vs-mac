#include "Lockscreen.h"
#include "LockscreenView.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IResourceManager.h"

namespace game::scene
{
	Lockscreen::Lockscreen(core::iface::IInputProvider& inputProvider,
		core::iface::IUIRenderer& uiRenderer,
		core::iface::IScreen& screen)
		: m_inputProvider{ inputProvider }
		, m_uiRenderer{ uiRenderer }
		, m_screen{ screen }
	{
		auto* res{ core::base::ServiceLocator::get<core::iface::IResourceManager>() };
		std::string fontName{ res->getFontName("normal").value_or("") };
		m_lockscreenView = std::make_unique<LockscreenView>(uiRenderer, screen, *res, std::move(fontName));

		// 起動時はフェードインから開始
		m_fade = std::make_unique<ui::FadeTransition>(uiRenderer, screen, FADE_DURATION, true);
	}

	Lockscreen::~Lockscreen() noexcept = default;

	void Lockscreen::update(float deltaTime)
	{
		switch (m_state)
		{
		case State::FadeIn:
			m_fade->update(deltaTime);
			if (m_fade->isFinished())
			{
				m_fade = nullptr;
				m_state = State::Idle;
			}
			break;

		case State::Idle:
			m_lockscreenView->update(deltaTime);
			if (m_inputProvider.isKeyPressed(core::input::KeyCode::Space)
				|| m_inputProvider.isMouseLeftPressed()
				|| m_inputProvider.isMouseRightPressed())
			{
				m_state = State::Sliding;
			}
			break;

		case State::Sliding:
		{
			m_lockscreenView->update(deltaTime);
			const float slideSpeed{ static_cast<float>(m_screen.getHeight()) / SLIDE_DURATION };
			m_offsetY -= slideSpeed * deltaTime;
			if (m_offsetY <= -static_cast<float>(m_screen.getHeight()))
			{
				auto* sceneManager{ core::base::ServiceLocator::get<SceneManager>() };
				sceneManager->changeScene(SceneType::Title);
			}
			break;
		}
		}
	}

	void Lockscreen::draw()
	{
		m_lockscreenView->draw(static_cast<int>(m_offsetY));
		if (m_fade) m_fade->draw(m_uiRenderer, m_screen);
	}
}
