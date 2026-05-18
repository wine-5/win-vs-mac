#pragma once
#include "IScene.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IResultWindowManager.h"
#include "game/ui/UIManager.h"
#include <memory>

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
         * @brief リザルトウィンドウマネージャーを設定する
         * @param manager ウィンドウマネージャー（所有権を移譲）
         */
        void setWindowManager(std::unique_ptr<core::iface::IResultWindowManager> manager) noexcept;

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
         * @brief UIを構築する（フォールバック用）
         */
        void setupUI();

        core::iface::IInputProvider& m_inputProvider;
        core::iface::IUIRenderer&    m_uiRenderer;
        core::iface::IScreen&        m_screen;
        ui::UIManager                m_uiManager;

        std::unique_ptr<core::iface::IResultWindowManager> m_windowManager{};
    };
}