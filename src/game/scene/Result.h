#pragma once
#include "IScene.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "game/ui/UIManager.h"

namespace game::scene
{
    /**
     * @brief リザルトシーンのクラス
     */
    class Result : public IScene
    {
    public:
        /**
         * @brief Resultのコンストラクタ
         * @param inputProvider 入力インターフェース
         * @param uiRenderer UI描画インターフェース
         * @param screen 画面情報インターフェース
         */
        Result(core::iface::IInputProvider& inputProvider,
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
        /**
         * @brief UIを構築する
         */
        void setupUI();

        core::iface::IInputProvider& m_inputProvider;
        core::iface::IUIRenderer& m_uiRenderer;
        core::iface::IScreen& m_screen;
        ui::UIManager m_uiManager;
    };
}