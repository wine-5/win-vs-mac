#include "SceneFactory.h"
#include "Title.h"
#include "StageSelect.h"
#include "Loading.h"
#include "InGame.h"
#include "Result.h"
#include "core/ServiceLocator.h"
#include "core/interface/IFileProvider.h"
#include "game/GameManager.h"

namespace game::scene
{
    SceneFactory::SceneFactory()
        : m_inGameScene{}, m_titleScene{}, m_stageSelectScene{}, m_loadingScene{}, m_resultScene{}
    {
    }

    IScene *SceneFactory::createScene(SceneType sceneType)
    {
        // TODO: 現在は全て軽量なシーンであるためシーンが作られるたびに生成をしているが、
        // 本来であればreset()関数をInGameなどの状態をゲームループするたびにリセットする必要がある
        // オブジェクトに持たせる必要があり、逆にリセットが不要なシーンに関しては
        // if文で重複して何度も生成されないようにするべき
        auto *screen = core::ServiceLocator::get<core::iface::IScreen>();

        switch (sceneType)
        {
        case SceneType::Title:
            m_titleScene = std::make_unique<Title>(
                m_titleInputManager,
                m_titleUIRenderer,
                *screen);
            return m_titleScene.get();

        case SceneType::StageSelect:
        {
            auto *fileProvider = core::ServiceLocator::get<core::iface::IFileProvider>();
            auto *gameManager = core::ServiceLocator::get<game::GameManager>();

            m_stageSelectScene = std::make_unique<StageSelect>(
                m_stageSelectInputManager,
                m_stageSelectUIRenderer,
                *screen,
                *fileProvider,
                gameManager->getFileEquipmentData());
            return m_stageSelectScene.get();
        }

        case SceneType::Loading:
            m_loadingScene = std::make_unique<Loading>(
                m_loadingUIRenderer,
                *screen);
            return m_loadingScene.get();

        case SceneType::InGame:
        {
            auto *gameManager = core::ServiceLocator::get<game::GameManager>();

            m_inGameScene = std::make_unique<InGame>(
                m_inGameCamera,
                m_inGameRenderer,
                m_inGameAnimator,
                m_inGameResourceManager,
                m_inGameInputManager,
                gameManager->getFileEquipmentData());
            return m_inGameScene.get();
        }

        case SceneType::Result:
            m_resultScene = std::make_unique<Result>(
                m_resultInputManager,
                m_resultUIRenderer,
                *screen);
            return m_resultScene.get();

        default:
            return nullptr;
        }
    }
}