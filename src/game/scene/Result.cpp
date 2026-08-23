#include "Result.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "game/GameManager.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IAudioManager.h"
#include "core/constant/BgmType.h"

namespace game::scene
{
	Result::Result(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    std::unique_ptr<core::iface::IWindow> resultWindow,
	    GameManager& gameManager,
	    core::iface::IInputProvider& inputProvider)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_resultWindow{ std::move(resultWindow) }
	    , m_inputProvider{ inputProvider }
	    , m_inputMapper{ inputProvider }
	{
		const auto& resultData{ gameManager.getResultData() };

		// リザルトウィンドウを表示
        if (auto* resultWindowMgr = dynamic_cast<core::iface::IResultWindowManager*>(m_resultWindow.get()))
            resultWindowMgr->show(resultData);

        // リザルトBGM再生
        auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
        if (audio)
        {
            const auto bgmType{ resultData.m_isVictory ? core::constant::BgmType::ResultWin : core::constant::BgmType::ResultLose };
            audio->playBgm(bgmType, false);
        }
    }

    Result::~Result() noexcept = default;

	void Result::updateInput(float deltaTime)
	{
		auto* manager{ dynamic_cast<core::iface::IResultWindowManager*>(m_resultWindow.get()) };
		if (manager == nullptr)
			return;

		m_inputMapper.update(deltaTime);

		// 最初にパッドを触った時点で枠を出す。触るまで枠が出ていると、
		// マウスで遊ぶ人の画面に意味のない枠が残り続ける
		if (!m_hasPadFocus &&
		    m_inputProvider.getLastInputDevice() == core::input::InputDevice::GamePad)
		{
			m_hasPadFocus = true;
			manager->sendPadAction("focus");
		}

		if (m_inputMapper.isTriggered(ui::UiAction::NavigateUp))
			manager->sendPadAction("up");
		if (m_inputMapper.isTriggered(ui::UiAction::NavigateDown))
			manager->sendPadAction("down");
		if (m_inputMapper.isTriggered(ui::UiAction::NavigateLeft))
			manager->sendPadAction("left");
		if (m_inputMapper.isTriggered(ui::UiAction::NavigateRight))
			manager->sendPadAction("right");

		if (m_inputProvider.consumePadPress(core::input::GamePadCode::ButtonCross))
			manager->sendPadAction("confirm");
	}

	void Result::update(float /*deltaTime*/)
	{
		if (m_resultWindow)
            m_resultWindow->pumpMessages();
	}

	void Result::draw()
    {
    }
} // namespace game::scene
