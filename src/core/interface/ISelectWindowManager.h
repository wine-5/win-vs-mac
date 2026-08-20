#pragma once
#include <string>

namespace core::iface
{
	/**
	 * @brief セレクト画面のWindow管理のインターフェース
	 */
	class ISelectWindowManager
	{
	public:
		virtual ~ISelectWindowManager() = default;

		/**
		 * @brief すべてのWindowを作成する
		 */
		virtual void createAllWindows() = 0;

		/**
		 * @brief すべてのWindowを破棄する
		 */
		virtual void destroyAllWindows() = 0;

		/**
		 * @brief メッセージポンプ（毎フレーム呼び出し）
		 */
		virtual void pumpMessages() = 0;

		/**
		 * @brief 警告メッセージボックスを表示する
		 * @param message メッセージ内容
		 */
		virtual void showWarningMessage(const std::string& message) = 0;

		/**
		 * @brief パッドの操作を、いま操作対象になっているWindowへ送る
		 *
		 * セレクト画面の中身は WebView2 のHTMLで、DxLibの入力ループの外にある。
		 * OSのカーソルを合成で動かすとゲームが前面を失った瞬間に他のアプリへ
		 * 入力が漏れるため、代わりに「何をしたいか」をJSへ渡してフォーカスを動かす
		 * @param action 操作名（"up" / "down" / "left" / "right" / "confirm"）
		 */
		virtual void sendPadAction(const char* action) = 0;

		/**
		 * @brief パッドで操作する対象のWindowを切り替える
		 *
		 * セレクト画面は独立したWindowが複数枚並ぶ。ページ内の移動だけでは
		 * 隣のWindowへ渡れないため、切り替えは別の操作（L1/R1）に割り当てる
		 * @param delta +1で次、-1で前
		 */
		virtual void movePadWindowFocus(int delta) = 0;

		/**
		 * @brief すべてのWindowの表示/非表示をまとめて切り替える
		 *
		 * セレクト画面のWindowは常時最前面のため、ポーズメニューのような
		 * ゲーム本体側の描画が裏に隠れてしまう。開いている間は引っ込める
		 * @param visible 表示するならtrue
		 */
		virtual void setWindowsVisible(bool visible) = 0;
	};
} // namespace core::iface