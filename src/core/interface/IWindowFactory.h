#pragma once
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
		 * @param speedMultiplier 演出の再生速度倍率（1.0で等速）
		 * @return 生成されたローディングウィンドウ（既に初期化済み）
		 */
		virtual std::unique_ptr<IWindow> createLoadingWindow(
		    std::function<void()> onLoadingComplete,
		    float speedMultiplier) = 0;

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
		 * @param onBackToTitle タイトルへ戻るときのコールバック
		 * @param onFileSlotChanged ファイルスロット変更時のコールバック
		 * @param onDifficultyChanged 難易度変更時のコールバック（"NORMAL" | "HARD"）
		 * @param resourceManager リソースマネージャ
		 * @param showTutorial 初回の操作ガイドを表示するか
		 * @return 生成されたセレクトウィンドウマネージャ
		 */
		virtual std::unique_ptr<ISelectWindowManager> createSelectWindowManager(
		    std::function<void()> onGameStart,
		    std::function<void()> onBackToTitle,
		    std::function<void(int, const std::string&)> onFileSlotChanged,
		    std::function<void(const std::string&)> onDifficultyChanged,
		    IResourceManager& resourceManager,
		    bool showTutorial) = 0;
	};
} // namespace core::iface
