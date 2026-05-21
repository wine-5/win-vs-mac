#pragma once
#include <memory>
#include <functional>

namespace core::iface
{
    class IWindow;

    /**
     * @brief プラットフォーム層のウィンドウ生成を担当するインターフェース
     * @details Game層がプラットフォーム層の具体的な実装に依存しないようにする
     */
    class IWindowFactory
    {
    public:
        virtual ~IWindowFactory() = default;

        /**
         * @brief ローディングウィンドウを生成・初期化
         * @param onLoadingComplete ローディング完了時のコールバック
         * @return 生成されたローディングウィンドウ（既に初期化済み）
         */
        virtual std::unique_ptr<IWindow> createLoadingWindow(
            std::function<void()> onLoadingComplete) = 0;

        /**
         * @brief リザルトウィンドウを生成・初期化
         * @param onRetry リトライボタン押下時のコールバック
         * @param onTitle タイトルボタン押下時のコールバック
         * @return 生成されたリザルトウィンドウ（既に初期化済み）
         */
        virtual std::unique_ptr<IWindow> createResultWindow(
            std::function<void()> onRetry,
            std::function<void()> onTitle) = 0;
    };
}
