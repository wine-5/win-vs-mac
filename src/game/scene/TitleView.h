#pragma once
#include "game/ui/UIManager.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IPerformanceDataProvider.h"
#include <functional>
#include <array>
#include <string>

namespace game::ui
{
	class Button;
}

namespace game::scene
{
	/**
	 * @brief タイトルシーンの描画クラス
	 */
	class TitleView
	{
	public:
		/**
		 * @brief TitleViewのコンストラクタ
		 * @param inputProvider 入力インターフェース
		 * @param uiRenderer UI描画インターフェース
		 * @param screen 画面情報インターフェース
		 * @param mainFontName 使用するフォント名
		 * @param onGoToSelect 「選択画面へ」ボタン押下時コールバック
		 * @param onExit 「終了」ボタン押下時コールバック
		 */
		TitleView(core::iface::IInputProvider& inputProvider,
			core::iface::IUIRenderer& uiRenderer,
			core::iface::IScreen& screen,
			std::string mainFontName,
			std::function<void()> onGoToSelect,
			std::function<void()> onExit);

		/**
		 * @brief ボタン入力とパフォーマンス履歴を更新する
		 * @param snap 最新のパフォーマンススナップショット
		 */
		void update(const core::iface::PerformanceSnapshot& snap);

		/**
		 * @brief スプラッシュ画面を描画する
		 * @param dotCount 末尾に表示するドットの数（0〜3）
		 */
		void drawSplash(int dotCount) const;

		/**
		 * @brief タイトル画面を描画する
		 */
		void drawTitle() const;

		/**
		 * @brief ボタンの表示・非表示を設定する
		 * @param visible trueで表示
		 */
		void setButtonsVisible(bool visible);

	private:
		static constexpr int HISTORY_SIZE{ 120 };

		void drawBackground() const;
		void pushHistory(std::array<float, HISTORY_SIZE>& buf, float value);

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen&     m_screen;
		std::string               m_mainFontName;

		ui::UIManager m_uiManager;
		ui::Button*   m_startButton{};
		ui::Button*   m_exitButton{};


		std::array<float, HISTORY_SIZE> m_cpuHistory{};
		std::array<float, HISTORY_SIZE> m_memHistory{};
		std::array<float, HISTORY_SIZE> m_diskHistory{};

		static constexpr float TITLE_Y_RATIO         = 0.35f;
		static constexpr float START_BUTTON_Y_RATIO  = 0.50f;
		static constexpr float EXIT_BUTTON_Y_RATIO   = 0.62f;
		static constexpr float BUTTON_WIDTH_RATIO    = 0.15f;
		static constexpr float BUTTON_HEIGHT_RATIO   = 0.06f;
	};
}