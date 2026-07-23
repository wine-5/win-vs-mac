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
		 * @brief Titleのデストラクタ
		 */
		~Title() noexcept;

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

		float m_perfTimer{};

		static constexpr float FADE_DURATION         = 0.5f;
		static constexpr float PERF_UPDATE_INTERVAL  = 1.0f; // パフォーマンス取得の更新間隔（秒）
	};
} // namespace game::scene