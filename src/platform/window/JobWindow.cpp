#include <windows.h>
#include "JobWindow.h"
#include "core/constant/JobType.h"
#include "thirdparty/nlohmann/json.hpp"
#include <sstream>

namespace platform::window
{
	JobWindow::JobWindow(int x, int y, int width, int height) noexcept
		: WindowBase(L"JobWindowClass", L"Job Selection", x, y, width, height),
		m_selectedJob{core::constant::JobType::Warrior}
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
		try { m_jobRepository.emplace(); } catch (...) {}
		setIcon(hwnd, ICON_PATH);
		m_webView.setOnMessage([this](const std::string& json) noexcept {
			handleMessage(json);
		});
		m_webView.initialize(hwnd, L"https://game.web/job/job.html");
	}

	LRESULT JobWindow::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		if (msg == WM_SIZE)
		{
			if (wParam == SIZE_MINIMIZED)
				m_webView.setVisible(false);
			else
				m_webView.resize(LOWORD(lParam), HIWORD(lParam));
			return 0;
		}
		if (msg == WM_SHOWWINDOW)
			m_webView.setVisible(wParam != 0);
		if (msg == WM_ACTIVATEAPP && wParam != 0)
		{
			RECT rc{};
			GetClientRect(hwnd, &rc);
			if (rc.right > 0 && rc.bottom > 0)
				m_webView.resize(rc.right, rc.bottom);
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
			else if (type == "requestJobStats")
			{
				sendJobStats();
			}
		}
		catch (...) {}
	}

	void JobWindow::sendJobStats() noexcept
	{
		if (!m_jobRepository) return;
		try
		{
			nlohmann::json resp;
			resp["type"]  = "jobStats";
			resp["stats"] = nlohmann::json::array();
			for (int i{0}; i < core::constant::JOB_COUNT; ++i)
			{
				auto info = m_jobRepository->getJobInfo(static_cast<core::constant::JobType>(i));
				nlohmann::json s;
				s["id"]  = i;
				s["hp"]  = info.m_hp;
				s["atk"] = info.m_atk;
				s["def"] = info.m_def;
				s["spd"] = info.m_spd;
				resp["stats"].push_back(s);
			}
			m_webView.postMessage(resp.dump());
		}
		catch (...) {}
	}
}
