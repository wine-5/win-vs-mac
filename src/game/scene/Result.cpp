#include "Result.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "game/GameManager.h"
#include "core/base/ServiceLocator.h"

namespace game::scene
{
    Result::Result(core::iface::IUIRenderer& uiRenderer,
        core::iface::IScreen& screen,
        std::unique_ptr<core::iface::IWindow> resultWindow)
        : m_uiRenderer{ uiRenderer }
        , m_screen{ screen }
        , m_resultWindow{ std::move(resultWindow) }
    {
        // リザルトウィンドウを表示
        if (auto* resultWindowMgr = dynamic_cast<core::iface::IResultWindowManager*>(m_resultWindow.get()))
        {
            const auto& resultData = GameManager::getInstance().getResultData();
            resultWindowMgr->show(resultData);
        }
    }

    Result::~Result() noexcept = default;

    void Result::update(float deltaTime)
    {
        if (m_resultWindow)
            m_resultWindow->pumpMessages();
    }

    void Result::draw()
    {
    }
}
