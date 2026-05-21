#pragma once
#include "IScene.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <memory>

namespace platform::window::result
{
    class ResultWindow;
}

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
         */
        Result(core::iface::IUIRenderer& uiRenderer,
            core::iface::IScreen& screen);

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
        core::iface::IUIRenderer&    m_uiRenderer;
        core::iface::IScreen&        m_screen;

        std::unique_ptr<platform::window::result::ResultWindow> m_resultWindow;
    };
}