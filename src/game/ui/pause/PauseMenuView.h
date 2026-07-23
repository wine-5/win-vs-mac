#pragma once
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <vector>
#include <string>

namespace game::ui::pause
{
	enum class PauseMenuAction; // 前方宣言（PauseMenu.h で定義）

	/**
	 * @brief ポーズメニューの描画を担当するView
	 *
	 * 画面全体を半透明の黒で覆い、タイトルと項目リストを中央に描画する。
	 * 状態は持たず、項目と選択位置を都度受け取って描画する。
	 * マウスのヒット判定用に項目の矩形計算も担う（レイアウトの一元管理）。
	 */
	class PauseMenuView
	{
	  public:
		/**
		 * @brief PauseMenuViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 */
		PauseMenuView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen);

		/**
		 * @brief ポーズメニューを描画する
		 * @param items 表示する項目（上から順）
		 * @param selectedIndex 選択中の項目のインデックス
		 */
		void draw(const std::vector<PauseMenuAction>& items, int selectedIndex);

		/**
		 * @brief 指定座標の上にある項目のインデックスを返す（マウスホバー用）
		 * @param x 判定するX座標
		 * @param y 判定するY座標
		 * @param itemCount 表示中の項目数
		 * @return 座標上の項目のインデックス（どれにも当たらなければ-1）
		 */
		[[nodiscard]] int getItemIndexAt(int x, int y, int itemCount) const;

	  private:
		/**
		 * @brief 項目の矩形を計算する（描画とヒット判定で共有する）
		 * @param index 項目のインデックス
		 * @param outX 矩形左上X座標の出力先
		 * @param outY 矩形左上Y座標の出力先
		 * @param outWidth 矩形幅の出力先
		 * @param outHeight 矩形高さの出力先
		 */
		void getItemRect(int index, int& outX, int& outY, int& outWidth, int& outHeight) const;

		/**
		 * @brief 項目の表示ラベルを返す（Shift-JIS変換済み）
		 * @param action 項目の種類
		 * @return 描画に使うラベル文字列
		 */
		[[nodiscard]] std::string getLabel(PauseMenuAction action) const;

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
	};
} // namespace game::ui::pause
