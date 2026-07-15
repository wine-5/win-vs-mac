#pragma once
#include "IScene.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IWindow.h"
#include "core/interface/IResultWindowManager.h"
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
         * @param uiRenderer UI描画インターフェース
         * @param screen 画面情報インターフェース
         * @param resultWindow リザルトウィンドウ
         */
        Result(core::iface::IUIRenderer& uiRenderer,
            core::iface::IScreen& screen,
            std::unique_ptr<core::iface::IWindow> resultWindow);

        /**
         * @brief Resultのデストラクタ
         */
        ~Result() noexcept;

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
        core::iface::IUIRenderer&              m_uiRenderer;
        core::iface::IScreen&                  m_screen;
        std::unique_ptr<core::iface::IWindow> m_resultWindow{};
    };
} // namespace game::scene