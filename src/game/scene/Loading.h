#pragma once
#include "IScene.h"
#include "game/ui/FadeTransition.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IWindow.h"
#include "core/interface/IResourcePreloader.h"
#include <memory>

namespace game::scene
{
    /**
     * @brief ローディングシーンのクラス
     */
    class Loading : public IScene
    {
    public:
	  /**
	   * @brief Loadingのコンストラクタ
	   * @param uiRenderer UI描画インターフェース
	   * @param screen 画面情報インターフェース
	   * @param loadingWindow ローディングウィンドウ
	   * @param preloader リソース先読み（完了を待ってからInGameへ進むために参照する）
	   */
	  Loading(core::iface::IUIRenderer& uiRenderer,
		  core::iface::IScreen& screen,
		  std::unique_ptr<core::iface::IWindow> loadingWindow,
		  core::iface::IResourcePreloader& preloader);

	  /**
	   * @brief Loadingのデストラクタ
	   */
	  ~Loading() noexcept;

	  /**
	   * @brief シーンの更新処理
	   * @param deltaTime フレーム間の時間差
	   */
	  void update(float deltaTime) override;

	  /**
	   * @brief シーンの描画処理
	   */
	  void draw() override;

	  /**
	   * @brief ローディング演出の完了通知（LoadingWindowからのコールバック用）
	   *
	   * 演出が終わっても先読みが残っている場合はここでは進まず、update()側で待つ。
	   */
	  void notifyLoadingComplete() noexcept;

    private:
        enum class State
        {
            FadeIn,
            Loading,
            FadeOut
        };

        void startFadeOut() noexcept;

        core::iface::IUIRenderer& m_uiRenderer;
        core::iface::IScreen& m_screen;
        std::unique_ptr<core::iface::IWindow> m_loadingWindow{};
		core::iface::IResourcePreloader& m_preloader;
		std::unique_ptr<ui::FadeTransition> m_fade;

        State m_state{ State::FadeIn };

		// 演出と先読みは独立に終わるため、両方揃うまでInGameへ進まない
		bool m_isAnimationFinished{ false };

		static constexpr float FADE_DURATION = 0.5f;
    };
} // namespace game::scene