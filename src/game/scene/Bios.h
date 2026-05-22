#pragma once
#include "IScene.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <vector>

namespace game::scene
{
    /**
     * @brief BIOS 風のスタートアップ画面シーン
     * @details ゲーム起動時に表示する PC 起動シーケンス演出。
     *          全行表示後に自動でロック画面へ遷移する。ESC でスキップ可能。
     */
    class Bios : public IScene
    {
    public:
        /**
         * @brief Bios のコンストラクタ
         * @param inputProvider 入力インターフェース
         * @param uiRenderer    UI描画インターフェース
         * @param screen        画面情報インターフェース
         */
        Bios(core::iface::IInputProvider& inputProvider,
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
        void transitionToLockscreen() noexcept;

        static constexpr float AUTO_ADVANCE_DELAY = 0.9f;

        core::iface::IInputProvider& m_inputProvider;
        core::iface::IUIRenderer&    m_uiRenderer;
        core::iface::IScreen&        m_screen;

        float              m_elapsed{};
        int                m_visibleCount{};
        float              m_postAllElapsed{};
        bool               m_transitioning{};
        std::vector<float> m_lineTimestamps;
    };
}
