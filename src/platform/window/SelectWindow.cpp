#include <windows.h>
#include "SelectWindow.h"
#include "core/interface/ILogger.h"
#include "thirdparty/nlohmann/json.hpp"

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
		m_webView.setOnMessage([this](const std::string& json) noexcept {
			handleMessage(json);
		});
		m_webView.initialize(hwnd, L"https://game.web/select.html");
	}

	LRESULT SelectWindow::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		if (msg == WM_SIZE)
		{
			m_webView.resize(LOWORD(lParam), HIWORD(lParam));
			return 0;
		}
		return WindowBase::onMessage(hwnd, msg, wParam, lParam);
	}

	void SelectWindow::handleMessage(const std::string& json) noexcept
	{
		try
		{
			auto j = nlohmann::json::parse(json);
			const std::string type = j.value("type", "");

			auto parseDifficulty = [this](const std::string& diff)
			{
				if (diff == "EASY")       m_selectedDifficulty = Difficulty::Easy;
				else if (diff == "HARD")  m_selectedDifficulty = Difficulty::Hard;
				else                      m_selectedDifficulty = Difficulty::Normal;
			};

			if (type == "difficultyChanged")
			{
				parseDifficulty(j.value("difficulty", "NORMAL"));
			}
			else if (type == "startGame")
			{
				parseDifficulty(j.value("difficulty", "NORMAL"));
				if (m_onGameStart)
					m_onGameStart();
			}
		}
		catch (...) {}
	}
}
