#pragma once
#include "IScene.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <memory>

namespace game::scene
{
    class TitleView;

    /**
     * @brief タイトルシーンのクラス
     */
    class Title : public IScene
    {
    public:
        /**
         * @brief Titleのコンストラクタ
         * @param inputProvider 入力インターフェース
         * @param uiRenderer UI描画インターフェース
         * @param screen 画面情報インターフェース
         */
        Title(core::iface::IInputProvider& inputProvider,
            core::iface::IUIRenderer& uiRenderer,
            core::iface::IScreen& screen);

        /**
         * @brief シーンの更新処理
         * @param deltaTime フレーム間の時間差
         */
        void update(float deltaTime) override;

        /**
         * @brief シーンの描画処理
         */
        void draw() override;

    private:
        std::unique_ptr<TitleView> m_view;
    };
}