#include <windows.h>
#include <commdlg.h>
#include "FileSelectWindow.h"
#include "platform/utility/StringConverter.h"

namespace platform::window
{
	FileSelectWindow::FileSelectWindow(int x, int y, int width, int height) noexcept
		: WindowBase(L"FileSelectWindowClass", L"File Selection", x, y, width, height)
	{
	}

	void FileSelectWindow::setOnFileSlotChanged(std::function<void(int, const std::string&)> callback) noexcept
	{
		m_onFileSlotChanged = callback;
	}

	std::string FileSelectWindow::getFilePath(int slot) const noexcept
	{
		if (slot < 0 || slot >= SLOT_COUNT) return "";
		return m_filePaths[slot];
	}

	void FileSelectWindow::onCreateControls(HWND hwnd)
	{
		constexpr int buttonWidth{ 120 };
		constexpr int buttonHeight{ 30 };
		constexpr int startY{ 20 };
		constexpr int spacing{ 40 };

		// Slot 1ボタン作成
		// BS_PUSHBUTTON: 通常のプッシュボタン
		m_slotButtons[0] = CreateWindowW(
			L"BUTTON",
			L"Slot 1",
			WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			20, startY, buttonWidth, buttonHeight,
			hwnd, (HMENU)IDC_SLOT1_BUTTON, GetModuleHandleW(nullptr), nullptr
		);

		// Slot 2ボタン作成
		m_slotButtons[1] = CreateWindowW(
			L"BUTTON",
			L"Slot 2",
			WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			20, startY + spacing, buttonWidth, buttonHeight,
			hwnd, (HMENU)IDC_SLOT2_BUTTON, GetModuleHandleW(nullptr), nullptr
		);

		// Slot 3ボタン作成
		m_slotButtons[2] = CreateWindowW(
			L"BUTTON",
			L"Slot 3",
			WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			20, startY + spacing * 2, buttonWidth, buttonHeight,
			hwnd, (HMENU)IDC_SLOT3_BUTTON, GetModuleHandleW(nullptr), nullptr
		);
	}

	LRESULT FileSelectWindow::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		switch (msg)
		{
		case WM_COMMAND:
		{
			// Slot ボタンが押下されたら対応するファイルダイアログを開く
			int controlId = LOWORD(wParam);
			int notificationCode = HIWORD(wParam);

			if (notificationCode == BN_CLICKED)
			{
				switch (controlId)
				{
				case IDC_SLOT1_BUTTON:
					openFileDialog(0);
					return 0;
				case IDC_SLOT2_BUTTON:
					openFileDialog(1);
					return 0;
				case IDC_SLOT3_BUTTON:
					openFileDialog(2);
					return 0;
				}
			}
			break;
		}
		default:
			break;
		}

		return WindowBase::onMessage(hwnd, msg, wParam, lParam);
	}

	void FileSelectWindow::openFileDialog(int slotIndex)
	{
		// OPENFILENAMEA: ファイルダイアログの構造体（A=ANSI版）
		OPENFILENAMEA ofn{};
		char szFile[260]{};  // ファイルパスを格納するバッファ

		// ファイルダイアログの設定
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = getHwnd();  // オーナーウィンドウ
		ofn.lpstrFile = szFile;  // 選択されたファイルパスが格納される
		ofn.nMaxFile = sizeof(szFile);  // バッファサイズ
		ofn.lpstrFilter = "All Files\0*.*\0";  // ファイルフィルタ
		ofn.nFilterIndex = 1;  // デフォルトフィルタインデックス
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;  // パス・ファイルが存在するかチェック

		// GetOpenFileNameA: Windows標準のファイル選択ダイアログ表示（モーダル）
		// 返り値が非ゼロならユーザーがファイルを選択した
		if (GetOpenFileNameA(&ofn))
		{
			m_filePaths[slotIndex] = szFile;
			if (m_onFileSlotChanged)
				m_onFileSlotChanged(slotIndex, m_filePaths[slotIndex]);
		}
	}
}
