#include "SceneFactory.h"
#include "Title.h"
#include "InGame.h"
#include "core/ServiceLocator.h"

namespace game::scene
{
    SceneFactory::SceneFactory()
        : m_inGameScene{}
        , m_titleScene{}
    {
    }

    IScene* SceneFactory::createScene(SceneType sceneType)
    {
        auto* screen = core::ServiceLocator::get<core::iface::IScreen>();

        switch (sceneType)
        {
        case SceneType::Title:
            // 初回のみ生成
            if (!m_titleScene)
            {
                m_titleScene = std::make_unique<Title>(
                    m_titleInputManager,
                    m_titleUIRenderer,
                    *screen);
            }
            return m_titleScene.get();

        case SceneType::InGame:
            // 初回のみ生成
            if (!m_inGameScene)
            {
                m_inGameScene = std::make_unique<InGame>(
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