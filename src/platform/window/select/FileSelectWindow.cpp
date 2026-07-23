#include <windows.h>
#include <commdlg.h>
#include <sstream>
#include "FileSelectWindow.h"
#include "platform/window/WindowConstants.h"
#include "core/interface/ILogger.h"
#include "thirdparty/nlohmann/json.hpp"
#include "core/utility/Log.h"
#include <exception>

namespace platform::window::select
{
	FileSelectWindow::FileSelectWindow(int x, int y, int width, int height) noexcept
	    : WebViewWindowBase(WINDOW_CLASS_NAME, WINDOW_TITLE, x, y, width, height)
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
		m_webView.initialize(hwnd, FILE_SELECT_HTML_URL);
	}

	LRESULT FileSelectWindow::onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		// サイズ追従・可視追従は WebViewWindowBase に集約している
		if (const auto handled{ handleWebViewMessage(hwnd, msg, wParam, lParam) })
			return *handled;

		return WindowBase::onMessage(hwnd, msg, wParam, lParam);
	}

	void FileSelectWindow::handleMessage(const std::string& json) noexcept
	{
		try
		{
			auto j = nlohmann::json::parse(json);
			const std::string type = j.value(platform::window::WindowConstants::JSON_KEY_TYPE, "");
			if (type == platform::window::WindowConstants::MESSAGE_TYPE_SLOT_SELECTED)
			{
				int slot = j.value("slot", 0);
				if (slot >= 0 && slot < SLOT_COUNT)
					openFileDialog(slot);
			}
			else if (type == platform::window::WindowConstants::MESSAGE_TYPE_REQUEST_BONUS_INFO)
			{
				sendBonusInfo();
			}
		}
		catch (const std::exception& e)
		{
			core::log::error("FileSelectWindow::handleMessage: 処理に失敗しました: {}", e.what());
		}
		catch (...)
		{
			core::log::error("FileSelectWindow::handleMessage: 不明な例外が発生しました");
		}
	}

	void FileSelectWindow::openFileDialog(int slotIndex)
	{
		OPENFILENAMEA ofn{};
		char szFile[MAX_PATH]{};

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = getHwnd();
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = FILE_DIALOG_FILTER;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn))
		{
			m_filePaths[slotIndex] = szFile;

			m_extensionTypes[slotIndex] = game::utility::FileExtensionTypeResolver::fromPath(m_filePaths[slotIndex]);

			if (m_onFileSlotChanged)
				m_onFileSlotChanged(slotIndex, m_filePaths[slotIndex]);

			sendSlotsRefresh();
		}
	}

	void FileSelectWindow::sendSlotsRefresh() noexcept
	{
		auto toName = [this](game::data::FileExtensionType t) -> const char* {
			switch (t)
			{
			case game::data::FileExtensionType::Executable: return EXT_TYPE_NAME_EXECUTABLE;
			case game::data::FileExtensionType::Document:   return EXT_TYPE_NAME_DOCUMENT;
			case game::data::FileExtensionType::Image:      return EXT_TYPE_NAME_IMAGE;
			case game::data::FileExtensionType::Audio:      return EXT_TYPE_NAME_AUDIO;
			case game::data::FileExtensionType::Archive:    return EXT_TYPE_NAME_ARCHIVE;
			default:                                        return EXT_TYPE_NAME_UNKNOWN;
			}
		};

		try
		{
			nlohmann::json resp;
			resp[platform::window::WindowConstants::JSON_KEY_TYPE]  = platform::window::WindowConstants::MESSAGE_TYPE_REFRESH;
			resp[platform::window::WindowConstants::JSON_KEY_FILE_SLOT] = nlohmann::json::array();
			for (int i = 0; i < SLOT_COUNT; ++i)
			{
				nlohmann::json s;
				s[platform::window::WindowConstants::JSON_KEY_FILE_SLOT] = i;
				if (m_filePaths[i].empty())
				{
					s[platform::window::WindowConstants::JSON_KEY_IS_EMPTY] = true;
				}
				else
				{
					s[platform::window::WindowConstants::JSON_KEY_IS_EMPTY] = false;
					std::string fileName{ m_filePaths[i] };
					auto slash = fileName.find_last_of("/\\");
					if (slash != std::string::npos)
						fileName = fileName.substr(slash + 1);
					s[platform::window::WindowConstants::JSON_KEY_FILE_NAME] = fileName;
					s[platform::window::WindowConstants::JSON_KEY_FILE_PATH] = m_filePaths[i];
					s[platform::window::WindowConstants::JSON_KEY_EXT_TYPE]  = toName(m_extensionTypes[i]);
				}
				resp[platform::window::WindowConstants::JSON_KEY_FILE_SLOT].push_back(s);
			}
			m_webView.postMessage(resp.dump());
		}
		catch (const std::exception& e)
		{
			core::log::error("FileSelectWindow::sendSlotsRefresh: 処理に失敗しました: {}", e.what());
		}
		catch (...)
		{
			core::log::error("FileSelectWindow::sendSlotsRefresh: 不明な例外が発生しました");
		}
	}

	void FileSelectWindow::sendBonusInfo() noexcept
	{
		// ExtensionBonusCalculator の定数から説明文を生成（C++ が正とする）
		struct Entry { const char* m_key; game::data::FileExtensionType m_type; };
		constexpr Entry ENTRIES[] = {
			{ EXT_TYPE_NAME_EXECUTABLE, game::data::FileExtensionType::Executable },
			{ EXT_TYPE_NAME_DOCUMENT, game::data::FileExtensionType::Document },
			{ EXT_TYPE_NAME_IMAGE, game::data::FileExtensionType::Image },
			{ EXT_TYPE_NAME_AUDIO, game::data::FileExtensionType::Audio },
			{ EXT_TYPE_NAME_ARCHIVE, game::data::FileExtensionType::Archive },
			{ EXT_TYPE_NAME_UNKNOWN, game::data::FileExtensionType::Unknown },
		};

		auto fmt = [](float v) -> std::string {
			if (v == static_cast<int>(v))
				return std::to_string(static_cast<int>(v));
			std::ostringstream oss{};
			oss << v;
			return oss.str();
		};

		auto describe = [&](game::data::FileExtensionType t) -> std::string {
			auto b = game::utility::ExtensionBonusCalculator::calculate(t);
			std::string result{};
			auto append = [&](const char* label, float val) {
				if (val == 0.0f) return;
				if (!result.empty()) result += ' ';
				result += label;
				result += '+';
				result += fmt(val);
			};
			append("HP",    b.hp);
			append("ATK",   b.atk);
			append("DEF",   b.def);
			append("SPD",   b.spd);
			append("Range", b.attackRange);
			return result;
		};

		try
		{
			nlohmann::json resp;
			resp[platform::window::WindowConstants::JSON_KEY_TYPE]  = platform::window::WindowConstants::MESSAGE_TYPE_BONUS_INFO;
			resp[platform::window::WindowConstants::JSON_KEY_DESCRIPTIONS] = nlohmann::json::object();
			for (const auto& e : ENTRIES)
				resp[platform::window::WindowConstants::JSON_KEY_DESCRIPTIONS][e.m_key] = describe(e.m_type);
			m_webView.postMessage(resp.dump());
		}
		catch (const std::exception& e)
		{
			core::log::error("FileSelectWindow::sendBonusInfo: 処理に失敗しました: {}", e.what());
		}
		catch (...)
		{
			core::log::error("FileSelectWindow::sendBonusInfo: 不明な例外が発生しました");
		}
	}
} // namespace platform::window::select
