#pragma once
#include "IScene.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"

namespace game::scene
{
    /**
     * @brief ローディングシーンのクラス
     */
    class Loading : public IScene
    {
    public:
        /**
         * @brief Loadingのコンストラクタ
         * @param uiRenderer UI描画インターフェース
         * @param screen 画面情報インターフェース
         */
        Loading(core::iface::IUIRenderer& uiRenderer,
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
        core::iface::IUIRenderer& m_uiRenderer;
        core::iface::IScreen& m_screen;
        float m_elapsedTime;

        static constexpr float TEST_LOADING_DURATION = 2.0f;  // TODO:一時的に秒数を指定しているが、本来であればWindowのエクスプローラーを検索などをしている時間にすること
    };
}