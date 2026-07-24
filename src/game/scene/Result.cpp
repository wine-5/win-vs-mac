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
	    GameManager& gameManager)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_resultWindow{ std::move(resultWindow) }
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

	void Result::update(float /*deltaTime*/)
	{
		if (m_resultWindow)
            m_resultWindow->pumpMessages();
	}

	void Result::draw()
    {
    }
} // namespace game::scene
