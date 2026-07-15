#pragma once
#include "core/constant/JobType.h"
#include <memory>
#include <functional>
#include <string>

namespace core::iface
{
    class IWindow;
    class ISelectWindowManager;
    class IResourceManager;

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

        /**
         * @brief セレクト画面のウィンドウマネージャを生成・初期化
         * @param onGameStart ゲーム開始時のコールバック
         * @param onJobSelect 職業選択時のコールバック
         * @param onFileSlotChanged ファイルスロット変更時のコールバック
         * @param resourceManager リソースマネージャ
         * @return 生成されたセレクトウィンドウマネージャ
         */
        virtual std::unique_ptr<ISelectWindowManager> createSelectWindowManager(
            std::function<void()> onGameStart,
            std::function<void(core::constant::JobType)> onJobSelect,
            std::function<void(int, const std::string&)> onFileSlotChanged,
            IResourceManager& resourceManager) = 0;
    };
} // namespace core::iface
