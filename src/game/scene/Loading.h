#pragma once
#include "IScene.h"
#include "game/ui/FadeTransition.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <memory>

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
         * @param nextScene ローディング完了後に遷移するシーン
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
        enum class State
        {
            Playing,
            FadingOut
        };

        core::iface::IUIRenderer& m_uiRenderer;
        core::iface::IScreen& m_screen;
        float m_elapsedTime{};
        State m_state{ State::Playing };
        std::unique_ptr<ui::FadeTransition> m_fade;

        static constexpr float TEST_LOADING_DURATION = 2.0f;  // TODO:一時的に秒数を指定しているが、本来であればWindowのエクスプローラーを検索などをしている時間にすること
        static constexpr float FADE_DURATION = 0.5f;
    };
}