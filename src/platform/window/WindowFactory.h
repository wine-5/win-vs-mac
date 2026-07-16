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

        /**
         * @brief セレクト画面のウィンドウマネージャを生成・初期化
         * @param onGameStart ゲーム開始時のコールバック
         * @param onJobSelect 職業選択時のコールバック
         * @param onFileSlotChanged ファイルスロット変更時のコールバック
         * @param resourceManager リソースマネージャ
         * @return 生成されたセレクトウィンドウマネージャ
         */
        std::unique_ptr<core::iface::ISelectWindowManager> createSelectWindowManager(
            std::function<void()> onGameStart,
            std::function<void(core::constant::JobType)> onJobSelect,
            std::function<void(int, const std::string&)> onFileSlotChanged,
            core::iface::IResourceManager& resourceManager) override;

		/**
		 * @brief 弾追従ウィンドウのマネージャを生成する
		 * @return 生成されたマネージャ
		 */
		std::unique_ptr<core::iface::IProjectileWindowManager> createProjectileWindowManager() override;

	  private:
        // ローディングウィンドウのサイズ比率（スクリーンサイズに対する割合）
        static constexpr int LOADING_WINDOW_SIZE_RATIO{ 80 };

        core::iface::IScreen& m_screen;
    };
} // namespace platform::window
