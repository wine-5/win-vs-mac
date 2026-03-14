#pragma once
#include "IScene.h"
#include "SceneType.h"
#include "infrastructure/Camera.h"
#include "infrastructure/Renderer.h"
#include "infrastructure/Animator.h"
#include "infrastructure/ResourceManager.h"
#include "infrastructure/InputManager.h"
#include "infrastructure/UIRenderer.h"
#include "InGameScene.h"
#include "TitleScene.h"
#include <memory>

namespace game::scene
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
        IScene* createScene(SceneType sceneType);

    private:
        // InGameScene用の依存関係と実体
        infrastructure::Camera m_inGameCamera;
        infrastructure::Renderer m_inGameRenderer;
        infrastructure::Animator m_inGameAnimator;
        infrastructure::ResourceManager m_inGameResourceManager;
        infrastructure::InputManager m_inGameInputManager;
        std::unique_ptr<InGameScene> m_inGameScene;

        // TitleScene用の依存関係と実体
        infrastructure::InputManager m_titleInputManager;
        infrastructure::UIRenderer m_titleUIRenderer;
        std::unique_ptr<TitleScene> m_titleScene;
    };
}