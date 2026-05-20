#pragma once
#include "IScene.h"
#include "game/ui/FadeTransition.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <memory>

namespace platform::window::loading
{
	class LoadingWindow;
}

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
         * @brief Loadingのデストラクタ
         */
        ~Loading() noexcept;

        /**
         * @brief シーンの更新処理
         * @param deltaTime フレーム間の時間差
         */
        void update(float deltaTime) override;

        /**
         * @brief シーンの描画処理
         */
        void draw() override;

        /**
         * @brief ローディング完了通知（LoadingWindowからのコールバック用）
         */
        void notifyLoadingComplete() noexcept;

    private:
        enum class State
        {
            FadeIn,
            Loading,
            FadeOut
        };

        void startFadeOut() noexcept;

        core::iface::IUIRenderer& m_uiRenderer;
        core::iface::IScreen& m_screen;
        std::unique_ptr<platform::window::loading::LoadingWindow> m_loadingWindow;
        std::unique_ptr<ui::FadeTransition> m_fade;

        State m_state{ State::FadeIn };

        static constexpr float FADE_DURATION = 0.5f;
    };
}