#include "Title.h"
#include "TitleView.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/ServiceLocator.h"
#include <cstdlib>

namespace game::scene
{
    Title::Title(core::iface::IInputProvider& inputProvider,
        core::iface::IUIRenderer& uiRenderer,
        core::iface::IScreen& screen)
        : m_view{std::make_unique<TitleView>(inputProvider, uiRenderer,screen)}
    {
    }

    void Title::update(float deltaTime)
    {
        m_view->update(deltaTime);

        if (!m_view->isReadyToChange()) return;

        switch (m_view->getNextAction())
        {
        case TitleView::Action::GoToStageSelect:
        {
            auto* sceneManager{ core::ServiceLocator::get<SceneManager>() };
            sceneManager->changeScene(SceneType::StageSelect);
            break;
        }
        case TitleView::Action::Exit:
            std::exit(0);
        default:
            break;
        }
    }

    void Title::draw()
    {
        m_view->draw();
    }
}