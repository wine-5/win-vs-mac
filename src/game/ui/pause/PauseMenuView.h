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
	 * Ctrl+Alt+Del を押したときのセキュリティオプション画面に見立てて描く。
	 *
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
		 * @param isPowerHovered 右下の電源ボタンにカーソルが乗っているか
		 */
		void draw(const std::vector<PauseMenuAction>& items, int selectedIndex, bool isPowerHovered);

		/**
		 * @brief 指定座標が右下の電源ボタンの上かを返す
		 * @param x 判定するX座標
		 * @param y 判定するY座標
		 * @return 電源ボタンの上なら true
		 */
		[[nodiscard]] bool isOnPowerButton(int x, int y) const;

		/**
		 * @brief 取り消せない操作の確認を描画する（メニューの上に重ねる）
		 * @param action 確認する操作
		 * @param isYesSelected 「はい」を選んでいるか
		 */
		void drawConfirm(PauseMenuAction action, bool isYesSelected);

		/**
		 * @brief 指定座標にある確認ボタンの番号を返す
		 * @param x 判定するX座標
		 * @param y 判定するY座標
		 * @return 0＝はい、1＝いいえ、どちらでもなければ -1
		 */
		[[nodiscard]] int getConfirmButtonAt(int x, int y) const;

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
		 * @brief 項目リストを描く
		 * @param items 表示する項目（上から順）
		 * @param selectedIndex 選択中の項目のインデックス
		 */
		void drawItems(const std::vector<PauseMenuAction>& items, int selectedIndex) const;

		/**
		 * @brief 右下に電源の記号を描く
		 * @param isHovered カーソルが乗っているか（乗っていれば明るくする）
		 */
		void drawPowerButton(bool isHovered) const;

		/**
		 * @brief 電源ボタンの円の位置と半径を返す（描画とヒット判定で共有する）
		 * @param outCenterX 中心X座標の出力先
		 * @param outCenterY 中心Y座標の出力先
		 * @param outRadius 半径の出力先
		 */
		void getPowerCircle(int& outCenterX, int& outCenterY, int& outRadius) const;

		/**
		 * @brief その位置が「キャンセル」かを返す
		 *
		 * 実物と同じく最後に置くので、末尾かどうかで判定する
		 * @param index 項目のインデックス
		 * @param itemCount 表示中の項目数
		 * @return キャンセルなら true
		 */
		[[nodiscard]] bool isCancelIndex(int index, int itemCount) const noexcept;

		/**
		 * @brief UTF-8の文字列を描画用（Shift-JIS）へ変換する
		 * @param utf8 変換する文字列
		 * @return 変換後の文字列
		 */
		[[nodiscard]] std::string getDrawableText(const char* utf8) const;

		/**
		 * @brief 項目の矩形を計算する（描画とヒット判定で共有する）
		 * @param index 項目のインデックス
		 * @param itemCount 表示中の項目数（キャンセルの前の間隔を空けるために使う）
		 * @param outX 矩形左上X座標の出力先
		 * @param outY 矩形左上Y座標の出力先
		 * @param outWidth 矩形幅の出力先
		 * @param outHeight 矩形高さの出力先
		 */
		void getItemRect(int index, int itemCount,
		    int& outX, int& outY, int& outWidth, int& outHeight) const;

		/**
		 * @brief 確認ダイアログの「はい」「いいえ」を描く
		 * @param isYesSelected 「はい」を選んでいるか
		 */
		void drawConfirmButtons(bool isYesSelected) const;

		/**
		 * @brief 確認ダイアログに出す説明文を返す（Shift-JIS変換済み）
		 * @param action 確認する操作
		 * @return 説明文
		 */
		[[nodiscard]] std::string getConfirmMessage(PauseMenuAction action) const;

		/**
		 * @brief 確認ダイアログの外枠の矩形を返す
		 * @param outX 左上X座標の出力先
		 * @param outY 左上Y座標の出力先
		 * @param outWidth 幅の出力先
		 * @param outHeight 高さの出力先
		 */
		void getConfirmPanelRect(int& outX, int& outY, int& outWidth, int& outHeight) const;

		/**
		 * @brief Shift_JISの文字列を、指定した幅に収まる行へ折り返す
		 *
		 * 日本語は語の間に空白が無いので、文字単位で見て入るところまで詰める。
		 * 行頭に句読点や閉じ括弧が来ると読みにくいので、それらは前の行へ残す
		 * @param text 折り返す文字列（Shift_JIS）
		 * @param maxWidth 1行に許す幅（ピクセル）
		 * @param fontSize 文字の大きさ
		 * @return 折り返した各行
		 */
		[[nodiscard]] std::vector<std::string> wrapText(
		    const std::string& text, int maxWidth, int fontSize) const;

		/**
		 * @brief 確認ダイアログのボタンの矩形を返す
		 * @param index 0＝はい、1＝いいえ
		 * @param outX 左上X座標の出力先
		 * @param outY 左上Y座標の出力先
		 * @param outWidth 幅の出力先
		 * @param outHeight 高さの出力先
		 */
		void getConfirmButtonRect(int index, int& outX, int& outY, int& outWidth, int& outHeight) const;

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
