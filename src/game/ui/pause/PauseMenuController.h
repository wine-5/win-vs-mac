#pragma once
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "game/ui/pause/PauseMenuView.h"
#include <vector>

namespace game::ui::pause
{
	/**
	 * @brief ポーズメニューで選択された操作
	 */
	enum class PauseMenuAction
	{
		None,        // 何も選択されていない
		Resume,      // ゲームに戻る
		BackToTitle, // タイトルへ戻る
		Quit,        // ゲームを終了する
	};

	/**
	 * @brief ポーズメニューの入力・選択状態を制御するコントローラ
	 *
	 * Application が所有し、シーンをまたいで使い回す。
	 * キーボード（↑↓で移動・Enterで決定）とマウス（ホバーで選択・左クリックで決定）の
	 * 入力を捌いて選択状態を更新し、描画は PauseMenuView へ委譲する。
	 */
	class PauseMenuController
	{
	  public:
		/**
		 * @brief PauseMenuControllerのコンストラクタ
		 * @param inputProvider 入力のインターフェース
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 */
		PauseMenuController(core::iface::IInputProvider& inputProvider,
		    core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen);

		/**
		 * @brief メニューを開く（項目リストと選択位置を初期化する）
		 * @param allowBackToTitle 「タイトルへ戻る」を表示するかどうか
		 */
		void open(bool allowBackToTitle);

		/**
		 * @brief 入力を処理し、決定された操作を返す
		 * @return 決定された操作（未決定なら PauseMenuAction::None）
		 */
		[[nodiscard]] PauseMenuAction update();

		/**
		 * @brief メニューを描画する
		 */
		void draw();

	  private:
		core::iface::IInputProvider& m_inputProvider;
		core::iface::IScreen& m_screen;
		PauseMenuView m_view;

		std::vector<PauseMenuAction> m_items{}; // 表示中の項目（上から順）
		int m_selectedIndex{ 0 };
		bool m_prevMouseLeft{ false }; // マウス左クリックのエッジ検出用
	};
} // namespace game::ui::pause
