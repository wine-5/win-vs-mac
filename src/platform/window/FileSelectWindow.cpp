#include <windows.h>
#include <commdlg.h>
#include "FileSelectWindow.h"
#include "core/interface/ILogger.h"
#include "thirdparty/nlohmann/json.hpp"

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
		setIcon(hwnd, ICON_PATH);
		m_webView.setOnMessage([this](const std::string& json) noexcept {
			handleMessage(json);
		});
		m_webView.initialize(hwnd, L"https://game.web/file/file.html");
	}

	LRESULT FileSelectWindow::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
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

	void FileSelectWindow::handleMessage(const std::string& json) noexcept
	{
		try
		{
			auto j = nlohmann::json::parse(json);
			if (j.value("type", "") == "openFileDialog")
			{
				int slot = j.value("slot", 0);
				if (slot >= 0 && slot < SLOT_COUNT)
					openFileDialog(slot);
			}
		}
		catch (...) {}
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
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn))
		{
			m_filePaths[slotIndex] = szFile;
			if (m_onFileSlotChanged)
				m_onFileSlotChanged(slotIndex, m_filePaths[slotIndex]);

			// JS 側にファイルパスを通知
			nlohmann::json resp;
			resp["type"] = "fileSelected";
			resp["slot"] = slotIndex;
			resp["path"] = m_filePaths[slotIndex];
			m_webView.postMessage(resp.dump());
		}
	}
}
