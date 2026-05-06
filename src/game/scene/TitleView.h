#pragma once
#include "SceneType.h"
#include "game/ui/UIManager.h"
#include "game/ui/FadeTransition.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <memory>
#include <string>

namespace game::ui
{
	class Button;
}

namespace game::scene
{
	/**
	 * @brief タイトルシーンの描画・UI管理クラス
	 */
	class TitleView
	{
	public:
		enum class Action
		{
			None,
			GoToStageSelect,
			Exit
		};

		/**
		 * @brief TitleViewのコンストラクタ
		 * @param inputProvider 入力インターフェース
		 * @param uiRenderer UI描画インターフェース
		 * @param screen 画面サイズインターフェース
		 */
		TitleView(core::iface::IInputProvider& inputProvider,
			core::iface::IUIRenderer& uiRenderer,
			core::iface::IScreen& screen);

		/**
		 * @brief 更新処理
		 * @param deltaTime 経過時間（秒）
		 */
		void update(float deltaTime);

		/**
		 * @brief 描画処理
		 */
		void draw();

		/**
		 * @brief シーン切替の準備ができているか
		 * @return フェードアウト完了時にtrue
		 */
		[[nodiscard]] bool isReadyToChange() const;

		/**
		 * @brief 次に行うアクションを取得する
		 * @return アクション種別
		 */
		[[nodiscard]] Action getNextAction() const;

	private:
		enum class State
		{
			Splash,
			SplashFadeOut,
			TitleFadeIn,
			Idle,
			FadingOut
		};

		void setupUI();

		void startFadeOut(Action action);

		/**
		 * @brief スプラッシュのドットアニメーション文字列を取得する
		 * @return "Win VS Max.exe を起動しています" + ドット
		 */
		[[nodiscard]] std::string getSplashText() const;

		/* メンバ変数 */

		core::iface::IInputProvider& m_inputProvider;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;

		ui::UIManager                       m_uiManager;
		std::unique_ptr<ui::FadeTransition> m_fade;

		ui::Button* m_startButton{};
		ui::Button* m_exitButton{};

		State  m_state{ State::Splash };
		float  m_splashTimer{};
		float  m_dotTimer{};
		int    m_dotCount{};
		Action m_nextAction{ Action::None };

		static constexpr float SPLASH_DURATION = 3.0f;
		static constexpr float FADE_DURATION = 0.5f;
		static constexpr float DOT_INTERVAL = 0.4f;
		static constexpr int   MAX_DOTS = 3;

		static constexpr float TITLE_Y_RATIO = 0.35f;
		static constexpr float START_BUTTON_Y_RATIO = 0.50f;
		static constexpr float EXIT_BUTTON_Y_RATIO = 0.62f;
		static constexpr float BUTTON_WIDTH_RATIO = 0.15f;
		static constexpr float BUTTON_HEIGHT_RATIO = 0.06f;
	};
}