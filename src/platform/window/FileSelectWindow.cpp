#include <windows.h>
#include <commdlg.h>
#include "FileSelectWindow.h"
#include "platform/utility/StringConverter.h"
#include "core/interface/ILogger.h"

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
		RECT clientRect{};
		GetClientRect(hwnd, &clientRect);
		int windowWidth{ clientRect.right - clientRect.left };
		int windowHeight{ clientRect.bottom - clientRect.top };

		int buttonWidth{ windowWidth * 80 / 100 };
		int buttonHeight{ windowHeight * 20 / 100 };
		int startY{ windowHeight * 5 / 100 };
		int spacing{ windowHeight * 30 / 100 };
		int startX{ windowWidth * 10 / 100 };

		m_slotButtons[0] = CreateWindowW(
			L"BUTTON",
			L"Slot 1",
			WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			startX, startY, buttonWidth, buttonHeight,
			hwnd, (HMENU)IDC_SLOT1_BUTTON, GetModuleHandleW(nullptr), nullptr
		);
		LOG(m_slotButtons[0] ? "[FileSelectWindow] Slot 1 button created successfully" : "[FileSelectWindow] Slot 1 button FAILED to create");

		m_slotButtons[1] = CreateWindowW(
			L"BUTTON",
			L"Slot 2",
			WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			startX, startY + spacing, buttonWidth, buttonHeight,
			hwnd, (HMENU)IDC_SLOT2_BUTTON, GetModuleHandleW(nullptr), nullptr
		);
		OutputDebugStringW(m_slotButtons[1] ? L"[FileSelectWindow] Slot 2 button created successfully\n" : L"[FileSelectWindow] Slot 2 button FAILED to create\n");

		m_slotButtons[2] = CreateWindowW(
			L"BUTTON",
			L"Slot 3",
			WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			startX, startY + spacing * 2, buttonWidth, buttonHeight,
			hwnd, (HMENU)IDC_SLOT3_BUTTON, GetModuleHandleW(nullptr), nullptr
		);
		OutputDebugStringW(m_slotButtons[2] ? L"[FileSelectWindow] Slot 3 button created successfully\n" : L"[FileSelectWindow] Slot 3 button FAILED to create\n");
	}

	LRESULT FileSelectWindow::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		switch (msg)
		{
		case WM_COMMAND:
		{
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
		OPENFILENAMEA ofn{};
		char szFile[260]{};

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = getHwnd();
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "All Files\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameA(&ofn))
		{
			m_filePaths[slotIndex] = szFile;
			if (m_onFileSlotChanged)
				m_onFileSlotChanged(slotIndex, m_filePaths[slotIndex]);
		}
	}
}
