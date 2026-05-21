#include "Result.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "game/GameManager.h"
#include "core/base/ServiceLocator.h"
#include "platform/window/result/ResultWindow.h"

namespace game::scene
{
    Result::Result(core::iface::IUIRenderer& uiRenderer,
        core::iface::IScreen& screen)
        : m_uiRenderer{ uiRenderer }
        , m_screen{ screen }
    {
        m_resultWindow = std::make_unique<platform::window::result::ResultWindow>(
            screen,
            []() {
                auto* sceneManager = core::base::ServiceLocator::get<game::scene::SceneManager>();
                if (sceneManager)
                    sceneManager->changeScene(SceneType::Select);
            },
            []() {
                auto* sceneManager = core::base::ServiceLocator::get<game::scene::SceneManager>();
                if (sceneManager)
                    sceneManager->changeScene(SceneType::Title);
            }
        );

        const auto& resultData{ game::GameManager::getInstance().getResultData() };
        m_resultWindow->show(resultData);
    }

    Result::~Result() noexcept = default;

    void Result::update(float deltaTime)
    {
        m_resultWindow->pumpMessages();
    }

    void Result::draw()
    {
    }
}
