#pragma once
#include "core/interface/IWindowFactory.h"
#include "core/interface/IScreen.h"
#include <memory>
#include <functional>

namespace platform::window
{
    /**
     * @brief プラットフォーム層のウィンドウ生成の実装
     */
    class WindowFactory : public core::iface::IWindowFactory
    {
    public:
        /**
         * @brief コンストラクタ
         * @param screen 画面インターフェース
         */
        explicit WindowFactory(core::iface::IScreen& screen);

        /**
         * @brief ローディングウィンドウを生成・初期化
         * @param onLoadingComplete ローディング完了時のコールバック
         * @return 生成されたローディングウィンドウ（既に初期化済み）
         */
        std::unique_ptr<core::iface::IWindow> createLoadingWindow(
            std::function<void()> onLoadingComplete) override;

        /**
         * @brief リザルトウィンドウを生成・初期化
         * @param onRetry リトライボタン押下時のコールバック
         * @param onTitle タイトルボタン押下時のコールバック
         * @return 生成されたリザルトウィンドウ（既に初期化済み）
         */
        std::unique_ptr<core::iface::IWindow> createResultWindow(
            std::function<void()> onRetry,
            std::function<void()> onTitle) override;

    private:
        core::iface::IScreen& m_screen;
    };
}
