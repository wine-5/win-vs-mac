#include "Title.h"
#include "TitleView.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/ServiceLocator.h"
#include "core/interface/IResourceManager.h"
#include <cstdlib>

namespace game::scene
{
    Title::Title(core::iface::IInputProvider& inputProvider,
        core::iface::IUIRenderer& uiRenderer,
        core::iface::IScreen& screen)
    {
        auto* res{ core::ServiceLocator::get<core::iface::IResourceManager>() };
        std::string mainFontName{ res->getFontName("main").value_or("") };
        m_view = std::make_unique<TitleView>(inputProvider, uiRenderer, screen,
            std::move(mainFontName));
    }

    void Title::update(float deltaTime)
    {
        m_view->update(deltaTime);

        if (!m_view->isReadyToChange()) return;

        switch (m_view->getNextAction())
        {
        case TitleView::Action::GoToSelect:
        {
            auto* sceneManager{ core::ServiceLocator::get<SceneManager>() };
            sceneManager->changeScene(SceneType::Select);
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