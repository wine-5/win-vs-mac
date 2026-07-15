#pragma once
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"

namespace game::ui
{
    /**
     * @brief フェードイン・フェードアウト遷移
     */
    class FadeTransition
    {
    public:
        /**
         * @brief FadeTransitionのコンストラクタ
         * @param uiRenderer UI描画インターフェース
         * @param screen 画面サイズインターフェース
         * @param duration フェードにかかる時間（秒）
         * @param isFadeIn trueでフェードイン、falseでフェードアウト
         */
        FadeTransition(core::iface::IUIRenderer& uiRenderer,
            core::iface::IScreen& screen,
            float duration,
            bool isFadeIn);

        /**
         * @brief 遷移アニメーションを更新する
         * @param deltaTime 経過時間（秒）
         */
        void update(float deltaTime);

        /**
         * @brief 遷移アニメーションを描画する
         * @param uiRenderer UI描画インターフェース
         * @param screen 画面サイズインターフェース
         */
        void draw(core::iface::IUIRenderer& uiRenderer, core::iface::IScreen& screen) const;

        /**
         * @brief 遷移が完了したか
         * @return 完了した場合true
         */
        [[nodiscard]] bool isFinished() const;

    private:
        core::iface::IUIRenderer& m_uiRenderer;
        core::iface::IScreen& m_screen;
        float m_duration{};
        float m_elapsed{};
        bool m_isFadeIn{};
    };
} // namespace game::ui