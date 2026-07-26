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
		 * @brief すべてのWindowの表示/非表示をまとめて切り替える
		 *
		 * セレクト画面のWindowは常時最前面のため、ポーズメニューのような
		 * ゲーム本体側の描画が裏に隠れてしまう。開いている間は引っ込める
		 * @param visible 表示するならtrue
		 */
		virtual void setWindowsVisible(bool visible) = 0;
	};
} // namespace core::iface