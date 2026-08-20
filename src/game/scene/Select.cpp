#include "Select.h"
#include "core/input/GamePadCode.h"
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
	    std::unique_ptr<core::iface::ISelectWindowManager> windowManager,
	    core::iface::IInputProvider& inputProvider)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_resourceManager{ resourceManager }
	    , m_inputProvider{ inputProvider }
	    , m_inputMapper{ inputProvider }
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

	void Select::updateInput(float deltaTime)
	{
		if (m_windowManager == nullptr || m_state != State::Idle)
			return;

		m_inputMapper.update(deltaTime);

		using core::input::GamePadCode;

		// L1/R1 でWindowを渡り歩く。ページ内の移動だけでは隣のWindowへ行けない
		if (m_inputProvider.consumePadPress(GamePadCode::ButtonL1))
			m_windowManager->movePadWindowFocus(-1);
		if (m_inputProvider.consumePadPress(GamePadCode::ButtonR1))
			m_windowManager->movePadWindowFocus(1);

		// 最初にパッドを触った時点で枠を出す。触るまで枠が出ていると、
		// マウスで遊ぶ人の画面に意味のない枠が残り続ける
		if (!m_hasPadFocus &&
		    m_inputProvider.getLastInputDevice() == core::input::InputDevice::GamePad)
		{
			m_hasPadFocus = true;
			m_windowManager->sendPadAction("focus");
		}

		if (m_inputMapper.isTriggered(ui::UiAction::NavigateUp))
			m_windowManager->sendPadAction("up");
		if (m_inputMapper.isTriggered(ui::UiAction::NavigateDown))
			m_windowManager->sendPadAction("down");
		if (m_inputMapper.isTriggered(ui::UiAction::NavigateLeft))
			m_windowManager->sendPadAction("left");
		if (m_inputMapper.isTriggered(ui::UiAction::NavigateRight))
			m_windowManager->sendPadAction("right");

		if (m_inputProvider.consumePadPress(GamePadCode::ButtonCross))
			m_windowManager->sendPadAction("confirm");
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
				sceneManager->changeScene(m_nextScene);
			}
			break;
		}
	}

	void Select::draw()
	{
		if (m_fade)
			m_fade->draw();
	}

	void Select::onPauseChanged(bool isPaused)
	{
		if (m_windowManager)
			m_windowManager->setWindowsVisible(!isPaused);
	}

	void Select::startFadeOut(SceneType nextScene)
	{
		if (m_state == State::FadeOut)
		{
			return;
		}
		if (m_windowManager)
			m_windowManager->destroyAllWindows();
		m_nextScene = nextScene;
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
		startFadeOut(SceneType::Loading);
	}

	void Select::notifyBackToTitle() noexcept
	{
		if (!m_windowManager)
			return;
		startFadeOut(SceneType::Title);
	}

} // namespace game::scene
