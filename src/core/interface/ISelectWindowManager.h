#pragma once
#include "core/constant/SelectWindowId.h"
#include "core/constant/JobType.h"

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
		 * @brief 指定したWindowを前面に出す
		 * @param id Window識別子
		 */
		virtual void bringToFront(core::constant::SelectWindowId id) = 0;

		/**
		 * @brief 職業選択時にパラメータウィンドウを更新する
		 * @param jobType 職業タイプ
		 */
		virtual void updateParameterWindowForJob(core::constant::JobType jobType) = 0;

		/**
		 * @brief 職業が選択されているかを確認する
		 * @return 職業が選択されている場合true
		 */
		virtual bool isJobSelected() const = 0;

		/**
		 * @brief 警告メッセージボックスを表示する
		 * @param message メッセージ内容
		 */
		virtual void showWarningMessage(const std::string& message) = 0;
	};
}