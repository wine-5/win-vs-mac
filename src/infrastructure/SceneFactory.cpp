#include "SceneFactory.h"
#include "game/scene/TitleScene.h"
#include "game/scene/InGameScene.h"
#include "core/ServiceLocator.h"

namespace infrastructure
{
    SceneFactory::SceneFactory()
        : m_inGameScene(nullptr)
        , m_titleScene(nullptr)
    {
    }

    game::scene::IScene* SceneFactory::createScene(game::scene::SceneType sceneType)
    {
        auto* screen = core::ServiceLocator::get<core::iface::IScreen>();

        switch (sceneType)
        {
        case game::scene::SceneType::Title:
            // 初回のみ生成
            if (!m_titleScene)
            {
                m_titleScene = std::make_unique<game::scene::TitleScene>(
                    m_titleInputManager,
                    m_titleUIRenderer,
                    *screen);
            }
            return m_titleScene.get();

        case game::scene::SceneType::InGame:
            // 初回のみ生成
            if (!m_inGameScene)
            {
                m_inGameScene = std::make_unique<game::scene::InGameScene>(
                    m_inGameCamera,
                    m_inGameRenderer,
                    m_inGameAnimator,
                    m_inGameResourceManager,
                    m_inGameInputManager);
            }
            return m_inGameScene.get();

        default:
            return nullptr;
        }
    }
}