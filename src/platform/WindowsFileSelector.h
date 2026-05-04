#pragma once
#include "core/interface/IFileSelector.h"

namespace platform
{
	/**
	 * @brief Windows APIを使ったファイル選択クラス
	 */
	class WindowsFileSelector : public core::iface::IFileSelector
	{
	public:
		/**
		 * @brief Windowsファイル選択ダイアログを開く
		 * @return 選択されたファイルのフルパス。キャンセル時は空文字列を返す
		 */
		[[nodiscard]] std::string selectFile() override;
	};
}