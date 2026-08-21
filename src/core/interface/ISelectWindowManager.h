#pragma once
#include <string>
#include <functional>

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
		 * @brief 確認ダイアログを出している間に入力を回す処理を渡す
		 *
		 * 出撃やタイトルへ戻る前の確認は Win32 のダイアログで、自前のモーダルループを
		 * 回す。その間はゲームのループが止まるため、パッドを読む処理も動かなくなり、
		 * カーソルが1ミリも動かせなくなる。
		 *
		 * ダイアログを出している間だけタイマーからこれを呼ぶことで、
		 * 止まっている間もカーソルを動かせるようにする。パッドの解釈はGame層に残す
		 * @param pump 1回ぶん入力を回す処理（引数は前回からの経過秒数）
		 */
		virtual void setModalInputPump(std::function<void(float)> pump) = 0;

		/**
		 * @brief 警告メッセージボックスを表示する
		 * @param message メッセージ内容
		 */
		virtual void showWarningMessage(const std::string& message) = 0;

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