#include <windows.h>
#include "JobWindow.h"
#include "platform/window/WindowConstants.h"
#include "core/constant/JobType.h"
#include "thirdparty/nlohmann/json.hpp"
#include <sstream>

namespace platform::window::select
{
	JobWindow::JobWindow(int x, int y, int width, int height) noexcept
		: WindowBase(WINDOW_CLASS_NAME, WINDOW_TITLE, x, y, width, height),
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
		m_webView.initialize(hwnd, JOB_HTML_URL);
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
			const std::string type = j.value(platform::window::WindowConstants::JSON_KEY_TYPE, "");

			if (type == platform::window::WindowConstants::MESSAGE_TYPE_JOB_SELECTED)
			{
				const std::string job = j.value(platform::window::WindowConstants::JSON_KEY_JOB, JOB_NAME_WARRIOR);
				if (job == JOB_NAME_WARRIOR)    m_selectedJob = core::constant::JobType::Warrior;
				else if (job == JOB_NAME_MAGE)  m_selectedJob = core::constant::JobType::Mage;
				else if (job == JOB_NAME_NINJA) m_selectedJob = core::constant::JobType::Ninja;

				if (m_onJobSelect)
					m_onJobSelect(m_selectedJob);
			}
			else if (type == platform::window::WindowConstants::MESSAGE_TYPE_REQUEST_JOB_STATS)
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
			resp[platform::window::WindowConstants::JSON_KEY_TYPE]  = platform::window::WindowConstants::MESSAGE_TYPE_JOB_STATS;
			resp[platform::window::WindowConstants::JSON_KEY_STATS] = nlohmann::json::array();
			for (int i{0}; i < core::constant::JOB_COUNT; ++i)
			{
				auto info = m_jobRepository->getJobInfo(static_cast<core::constant::JobType>(i));
				nlohmann::json s;
				s[platform::window::WindowConstants::JSON_KEY_ID]  = i;
				s[platform::window::WindowConstants::JSON_KEY_HP]  = info.m_hp;
				s[platform::window::WindowConstants::JSON_KEY_ATK] = info.m_atk;
				s[platform::window::WindowConstants::JSON_KEY_DEF] = info.m_def;
				s[platform::window::WindowConstants::JSON_KEY_SPD] = info.m_spd;
				resp[platform::window::WindowConstants::JSON_KEY_STATS].push_back(s);
			}
			m_webView.postMessage(resp.dump());
		}
		catch (...) {}
	}
} // namespace platform::window::select
