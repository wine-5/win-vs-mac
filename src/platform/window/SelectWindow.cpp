#include <windows.h>
#include "SelectWindow.h"
#include "core/interface/ILogger.h"

namespace platform::window
{
	SelectWindow::SelectWindow(int x, int y, int width, int height) noexcept
		: WindowBase(L"SelectWindowClass", L"Difficulty Selection", x, y, width, height)
	{
	}

	void SelectWindow::setOnGameStart(std::function<void()> callback) noexcept
	{
		m_onGameStart = callback;
	}

	int SelectWindow::getSelectedDifficulty() const noexcept
	{
		return static_cast<int>(m_selectedDifficulty);
	}

	void SelectWindow::onCreateControls(HWND hwnd)
	{
		RECT clientRect{};
		GetClientRect(hwnd, &clientRect);
		int windowWidth{ clientRect.right - clientRect.left };
		int windowHeight{ clientRect.bottom - clientRect.top };

		int buttonWidth{ windowWidth * 40 / 100 };
		int buttonHeight{ windowHeight * 10 / 100 };
		int startY{ windowHeight * 5 / 100 };
		int spacing{ windowHeight * 15 / 100 };
		int startX{ windowWidth * 10 / 100 };

		m_easyButton = CreateWindowW(
			L"BUTTON",
			L"EASY",
			WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
			startX, startY, buttonWidth, buttonHeight,
			hwnd, (HMENU)IDC_EASY_BUTTON, GetModuleHandleW(nullptr), nullptr
		);

		m_normalButton = CreateWindowW(
			L"BUTTON",
			L"NORMAL",
			WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_TABSTOP,
			startX, startY + spacing, buttonWidth, buttonHeight,
			hwnd, (HMENU)IDC_NORMAL_BUTTON, GetModuleHandleW(nullptr), nullptr
		);
		::SendMessage(m_normalButton, BM_SETCHECK, BST_CHECKED, 0);

		m_hardButton = CreateWindowW(
			L"BUTTON",
			L"HARD",
			WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
			startX, startY + spacing * 2, buttonWidth, buttonHeight,
			hwnd, (HMENU)IDC_HARD_BUTTON, GetModuleHandleW(nullptr), nullptr
		);

		m_gameStartButton = CreateWindowW(
			L"BUTTON",
			L"Start Game",
			WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			startX, startY + spacing * 3 + windowHeight * 5 / 100, buttonWidth, buttonHeight,
			hwnd, (HMENU)IDC_GAME_START_BUTTON, GetModuleHandleW(nullptr), nullptr
		);
	}

	LRESULT SelectWindow::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
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
				case IDC_EASY_BUTTON:
					m_selectedDifficulty = Difficulty::Easy;
					return 0;
				case IDC_NORMAL_BUTTON:
					m_selectedDifficulty = Difficulty::Normal;
					return 0;
				case IDC_HARD_BUTTON:
					m_selectedDifficulty = Difficulty::Hard;
					return 0;
				case IDC_GAME_START_BUTTON:
					if (m_onGameStart)
						m_onGameStart();
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
}
