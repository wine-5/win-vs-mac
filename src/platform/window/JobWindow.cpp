#include <windows.h>
#include "JobWindow.h"
#include "core/constant/JobType.h"

namespace platform::window
{
	JobWindow::JobWindow(int x, int y, int width, int height) noexcept
		: WindowBase(L"JobWindowClass", L"Job Selection", x, y, width, height),
		m_selectedJob(core::constant::JobType::Warrior)
	{
	}

	void JobWindow::setOnJobSelect(std::function<void(core::constant::JobType)> callback) noexcept
	{
		m_onJobSelect = callback;
	}

	core::constant::JobType JobWindow::getSelectedJob() const noexcept
	{
		return m_selectedJob;
	}

	void JobWindow::onCreateControls(HWND hwnd)
	{
		RECT clientRect{};
		GetClientRect(hwnd, &clientRect);
		int windowWidth{ clientRect.right - clientRect.left };
		int windowHeight{ clientRect.bottom - clientRect.top };

		int buttonWidth{ windowWidth * 80 / 100 };
		int buttonHeight{ windowHeight * 15 / 100 };
		int startY{ windowHeight * 5 / 100 };
		int spacing{ windowHeight * 25 / 100 };
		int startX{ windowWidth * 10 / 100 };

		m_job1Button = CreateWindowW(
			L"BUTTON",
			L"Job 1",
			WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_TABSTOP,
			startX, startY, buttonWidth, buttonHeight,
			hwnd, (HMENU)IDC_JOB1_BUTTON, GetModuleHandleW(nullptr), nullptr
		);
		::SendMessage(m_job1Button, BM_SETCHECK, BST_CHECKED, 0);

		m_job2Button = CreateWindowW(
			L"BUTTON",
			L"Job 2",
			WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
			startX, startY + spacing, buttonWidth, buttonHeight,
			hwnd, (HMENU)IDC_JOB2_BUTTON, GetModuleHandleW(nullptr), nullptr
		);

		m_job3Button = CreateWindowW(
			L"BUTTON",
			L"Job 3",
			WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
			startX, startY + spacing * 2, buttonWidth, buttonHeight,
			hwnd, (HMENU)IDC_JOB3_BUTTON, GetModuleHandleW(nullptr), nullptr
		);
	}

	LRESULT JobWindow::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		switch (msg)
		{
		case WM_COMMAND:
		{
			int controlId = LOWORD(wParam);
			int notificationCode = HIWORD(wParam);

			if (notificationCode == BN_CLICKED)
			{
				core::constant::JobType selectedJob = m_selectedJob;

				switch (controlId)
				{
				case IDC_JOB1_BUTTON:
					selectedJob = core::constant::JobType::Warrior;
					break;
				case IDC_JOB2_BUTTON:
					selectedJob = core::constant::JobType::Mage;
					break;
				case IDC_JOB3_BUTTON:
					selectedJob = core::constant::JobType::Ninja;
					break;
				default:
					return WindowBase::onMessage(hwnd, msg, wParam, lParam);
				}

				m_selectedJob = selectedJob;
				if (m_onJobSelect)
					m_onJobSelect(m_selectedJob);

				return 0;
			}
			break;
		}
		default:
			break;
		}

		return WindowBase::onMessage(hwnd, msg, wParam, lParam);
	}
}
