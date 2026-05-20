#pragma once
#include "IScene.h"
#include "game/ui/FadeTransition.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <memory>

namespace core::iface
{
	class IPerformanceDataProvider;
}

namespace game::scene
{
	class TitleView;

	/**
	 * @brief タイトルシーンのクラス
	 */
	class Title : public IScene
	{
	public:
		/**
		 * @brief Titleのコンストラクタ
		 * @param inputProvider 入力インターフェース
		 * @param uiRenderer UI描画インターフェース
		 * @param screen 画面情報インターフェース
		 */
		Title(core::iface::IInputProvider& inputProvider,
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
		enum class State
		{
			Splash,
			SplashFadeOut,
			TitleFadeIn,
			Idle,
			FadingOut
		};

		void goToSelect();
		void exitApp();

		core::iface::IInputProvider&           m_inputProvider;
		core::iface::IUIRenderer&              m_uiRenderer;
		core::iface::IScreen&                  m_screen;
		core::iface::IPerformanceDataProvider* m_perfProvider{};

		std::unique_ptr<TitleView>          m_view{};
		std::unique_ptr<ui::FadeTransition> m_fade{};

		State m_state{ State::TitleFadeIn };

		float m_splashTimer{};
		float m_dotTimer{};
		int   m_dotCount{};

		static constexpr float SPLASH_DURATION = 3.0f;
		static constexpr float FADE_DURATION = 0.5f;
		static constexpr float DOT_INTERVAL = 0.4f;
		static constexpr int   MAX_DOTS = 3;
	};
}