#include "Select.h"
#include "core/input/GamePadCode.h"
#include <cmath>
#include "SceneManager.h"
#include "SceneType.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "core/interface/IAudioManager.h"
#include "core/constant/BgmType.h"

namespace
{
	// パッドでカーソルを動かす速さ（倒し切ったときのピクセル/秒）
	constexpr float POINTER_SPEED{ 1100.0f };

	/**
	 * @brief スティックの倒し量へ手前を緩やかにするカーブを掛ける
	 * @param value 倒し量（-1.0f〜1.0f）
	 * @return カーブを掛けた倒し量（-1.0f〜1.0f）
	 */
	float shapeStick(float value) noexcept
	{
		return value * std::abs(value);
	}
} // namespace

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
		if (m_state != State::Idle)
			return;

		using core::input::GamePadCode;

		// 左スティックでカーソルを動かす。倒し量をそのまま速さにすると細かく
		// 合わせられないので、2乗にして手前を緩やかにする
		const float stickX{ m_inputProvider.getPadAxis(GamePadCode::LeftStickX) };
		const float stickY{ m_inputProvider.getPadAxis(GamePadCode::LeftStickY) };

		if (stickX != 0.0f || stickY != 0.0f)
		{
			const float distance{ POINTER_SPEED * deltaTime };

			// 画面の縦は下が正。スティックは上が正なので符号を反転させる
			m_pointerRemainderX += shapeStick(stickX) * distance;
			m_pointerRemainderY += -shapeStick(stickY) * distance;

			// 整数ぶんだけ動かし、端数は次のフレームへ持ち越す
			const int moveX{ static_cast<int>(m_pointerRemainderX) };
			const int moveY{ static_cast<int>(m_pointerRemainderY) };
			m_pointerRemainderX -= static_cast<float>(moveX);
			m_pointerRemainderY -= static_cast<float>(moveY);

			m_inputProvider.movePointer(moveX, moveY);
		}
		else
		{
			// 倒していない間に端数を残すと、次に倒した瞬間に1ピクセル飛ぶ
			m_pointerRemainderX = 0.0f;
			m_pointerRemainderY = 0.0f;
		}

		if (m_inputProvider.consumePadPress(GamePadCode::ButtonCross))
			m_inputProvider.clickPointer();
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
