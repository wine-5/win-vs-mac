#include "WindowsFileSelector.h"
#include <Windows.h>

namespace platform
{
	std::string WindowsFileSelector::selectFile()
	{
		char filePath[MAX_PATH]; // Windowsが保証するパスの最大文字数
		filePath[0] = '\0'; // 静的解析の警告抑制（{}で全体がゼロ初期化済みだが明示する）

		OPENFILENAMEA ofn{};
		ofn.lStructSize = sizeof(ofn); // 構造体のサイズを設定する
		ofn.hwndOwner = nullptr; // nullptrにすることでデスクトップが親になる
		ofn.lpstrFilter = "すべてのファイル\0*.*\0";
		ofn.lpstrFile = filePath;
		ofn.nMaxFile = MAX_PATH;

		// OFN_FILEMUSTEXIST : 存在しないファイルを入力できないようにする
		// OFN_PATHMUSTEXIST : 存在しないフォルダパスを入力できないようにする
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

		// ダイアログを表示する
		if (GetOpenFileNameA(&ofn))
			return std::string{ filePath };

		// キャンセル時は空文字列を返す
		return {};
	}
}