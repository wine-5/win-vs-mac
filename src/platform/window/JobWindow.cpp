#include <windows.h>
#include "JobWindow.h"
#include "core/constant/JobType.h"
#include "thirdparty/nlohmann/json.hpp"

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
		m_webView.setOnMessage([this](const std::string& json) noexcept {
			handleMessage(json);
		});
		m_webView.initialize(hwnd, L"https://game.web/job.html");
	}

	LRESULT JobWindow::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		if (msg == WM_SIZE)
		{
			m_webView.resize(LOWORD(lParam), HIWORD(lParam));
			return 0;
		}
		return WindowBase::onMessage(hwnd, msg, wParam, lParam);
	}

	void JobWindow::handleMessage(const std::string& json) noexcept
	{
		try
		{
			auto j = nlohmann::json::parse(json);
			const std::string type = j.value("type", "");

			if (type == "jobSelected")
			{
				const std::string job = j.value("job", "Warrior");
				if (job == "Warrior")    m_selectedJob = core::constant::JobType::Warrior;
				else if (job == "Mage")  m_selectedJob = core::constant::JobType::Mage;
				else if (job == "Ninja") m_selectedJob = core::constant::JobType::Ninja;

				if (m_onJobSelect)
					m_onJobSelect(m_selectedJob);
			}
		}
		catch (...) {}
	}
}
