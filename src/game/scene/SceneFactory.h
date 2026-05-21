#pragma once
#include "IScene.h"
#include "SceneType.h"
#include "InGame.h"
#include "Title.h"
#include "Lockscreen.h"
#include "Select.h"
#include "Loading.h"
#include "Result.h"
#include "Bios.h"
#include "core/interface/ICamera.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IAnimator.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "core/interface/IWindowFactory.h"
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
         * @brief デストラクタ
         */
        ~SceneFactory();

        /**
         * @brief 指定されたタイプのシーンを生成
         * @param sceneType シーンの種類
         * @return 生成されたシーンへのポインタ（所有権はFactoryが保持）
         */
        IScene* createScene(SceneType sceneType);

        /**
         * @brief 指定されたタイプのシーンを破棄する
         * @details シーンが保持するウィンドウなどのリソースも合わせて解放する
         * @param sceneType 破棄するシーンの種類
         */
        void resetScene(SceneType sceneType) noexcept;

    private:
        // シーンインスタンスの管理
        std::unique_ptr<InGame>      m_inGameScene;
        std::unique_ptr<Title>       m_titleScene;
        std::unique_ptr<Lockscreen>  m_lockscreenScene;
        std::unique_ptr<Select>      m_selectScene;
        std::unique_ptr<Loading>     m_loadingScene;
        std::unique_ptr<Result>      m_resultScene;
        std::unique_ptr<Bios>        m_biosScene;
    };
}