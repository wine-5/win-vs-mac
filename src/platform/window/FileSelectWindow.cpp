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
			if (j.value("type", "") == "slotSelected")
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

			// Determine extension type
			std::string path{ szFile };
			auto dotPos = path.rfind('.');
			if (dotPos != std::string::npos)
			{
				std::string ext{ path.substr(dotPos) };
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
				m_extensionTypes[slotIndex] = game::utility::FileExtensionTypeResolver::toFileExtensionType(ext);
			}
			else
			{
				m_extensionTypes[slotIndex] = game::data::FileExtensionType::Unknown;
			}

			if (m_onFileSlotChanged)
				m_onFileSlotChanged(slotIndex, m_filePaths[slotIndex]);

			sendSlotsRefresh();
		}
	}

	void FileSelectWindow::sendSlotsRefresh() noexcept
	{
		auto toName = [](game::data::FileExtensionType t) -> const char* {
			switch (t)
			{
			case game::data::FileExtensionType::Executable: return "Executable";
			case game::data::FileExtensionType::Document:   return "Document";
			case game::data::FileExtensionType::Image:      return "Image";
			case game::data::FileExtensionType::Audio:      return "Audio";
			case game::data::FileExtensionType::Archive:    return "Archive";
			default:                                        return "Unknown";
			}
		};

		try
		{
			nlohmann::json resp;
			resp["type"]  = "refresh";
			resp["slots"] = nlohmann::json::array();
			for (int i = 0; i < SLOT_COUNT; ++i)
			{
				nlohmann::json s;
				s["slot"] = i;
				if (m_filePaths[i].empty())
				{
					s["isEmpty"] = true;
				}
				else
				{
					s["isEmpty"] = false;
					std::string fileName{ m_filePaths[i] };
					auto slash = fileName.find_last_of("/\\");
					if (slash != std::string::npos)
						fileName = fileName.substr(slash + 1);
					s["fileName"] = fileName;
					s["extType"]  = toName(m_extensionTypes[i]);
				}
				resp["slots"].push_back(s);
			}
			m_webView.postMessage(resp.dump());
		}
		catch (...) {}
	}
}
