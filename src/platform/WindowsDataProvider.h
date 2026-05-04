#pragma once
#include "core/interface/IFileProvider.h"

namespace platform
{
	/**
	 * @brief Windows API を使ってプラットフォーム情報を提供するクラス
	 */
	class WindowsDataProvider : public core::iface::IFileProvider
	{
	public:
		/**
		 * @brief Windowsファイル選択ダイアログを開く
		 * @return 選択されたファイルのフルパス。キャンセル時は空文字列を返す
		 */
		[[nodiscard]] std::string selectFile() override;
	};
}