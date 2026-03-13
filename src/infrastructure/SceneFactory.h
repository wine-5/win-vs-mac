#pragma once
#include "game/scene/IScene.h"
#include "game/scene/SceneType.h"
#include "Camera.h"
#include "Renderer.h"
#include "Animator.h"
#include "ResourceManager.h"
#include "InputManager.h"
#include "UIRenderer.h"
#include "game/scene/InGameScene.h"
#include "game/scene/TitleScene.h"
#include <memory>

namespace infrastructure
{
    /**
     * @brief シーンの生成を担当するファクトリークラス
     * 各シーンの依存関係を管理し、適切にシーンを生成する
     */
    class SceneFactory
    {
    public:
        SceneFactory();

        /**
         * @brief 指定されたタイプのシーンを生成
         * @param sceneType シーンの種類
         * @return 生成されたシーンへのポインタ（所有権はFactoryが保持）
         */
        game::scene::IScene* createScene(game::scene::SceneType sceneType);

    private:
        // InGameScene用の依存関係と実体
        Camera m_inGameCamera;
        Renderer m_inGameRenderer;
        Animator m_inGameAnimator;
        ResourceManager m_inGameResourceManager;
        InputManager m_inGameInputManager;
        std::unique_ptr<game::scene::InGameScene> m_inGameScene;

        // TitleScene用の依存関係と実体
        InputManager m_titleInputManager;
        UIRenderer m_titleUIRenderer;
        std::unique_ptr<game::scene::TitleScene> m_titleScene;
    };
}