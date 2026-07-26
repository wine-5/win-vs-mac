#include <windows.h>
#include <commdlg.h>
#include <sstream>
#include "FileSelectWindow.h"
#include "core/interface/IResourceManager.h"
#include "platform/window/WindowConstants.h"
#include "core/interface/ILogger.h"
#include "thirdparty/nlohmann/json.hpp"
#include "core/utility/Log.h"
#include <exception>
#include <utility>

namespace platform::window::select
{
	FileSelectWindow::FileSelectWindow(int x, int y, int width, int height,
	    core::iface::IResourceManager& resourceManager) noexcept
	    : WebViewWindowBase(WINDOW_CLASS_NAME, WINDOW_TITLE, x, y, width, height)
	    , m_resourceManager{ resourceManager }
	{
	}

	void FileSelectWindow::setOnFileSlotChanged(std::function<void(int, const std::string&)> callback) noexcept
	{
		m_onFileSlotChanged = std::move(callback);
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
			else if (type == platform::window::WindowConstants::MESSAGE_TYPE_REQUEST_SLOTS)
			{
				// ページが読み込み直された直後はJS側の装備状態が空に戻っている。
				// 装備そのものはC++が持ち続けているので、要求に応じて送り直す
				sendSlotsRefresh();
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
		auto toName = [this](core::data::FileExtensionType t) -> const char*
		{
			switch (t)
			{
			case core::data::FileExtensionType::Executable: return EXT_TYPE_NAME_EXECUTABLE;
			case core::data::FileExtensionType::Document: return EXT_TYPE_NAME_DOCUMENT;
			case core::data::FileExtensionType::Image: return EXT_TYPE_NAME_IMAGE;
			case core::data::FileExtensionType::Audio: return EXT_TYPE_NAME_AUDIO;
			case core::data::FileExtensionType::SourceCode: return EXT_TYPE_NAME_SOURCE_CODE;
			case core::data::FileExtensionType::Shortcut: return EXT_TYPE_NAME_SHORTCUT;
			case core::data::FileExtensionType::Video: return EXT_TYPE_NAME_VIDEO;
			case core::data::FileExtensionType::Archive: return EXT_TYPE_NAME_ARCHIVE;
			default:                                        return EXT_TYPE_NAME_UNKNOWN;
			}
		};

		try
		{
			nlohmann::json resp;
			resp[platform::window::WindowConstants::JSON_KEY_TYPE]  = platform::window::WindowConstants::MESSAGE_TYPE_REFRESH;
			// 配列は "slots"、要素内のスロット番号は "slot"。JS側（file-logic.js）が
			// data.slots / info.slot の組で読むため、ここを取り違えると一覧が更新されない
			resp[platform::window::WindowConstants::JSON_KEY_FILE_SLOTS] = nlohmann::json::array();
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
				resp[platform::window::WindowConstants::JSON_KEY_FILE_SLOTS].push_back(s);
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
		// extensionBonus.json の値から説明文を生成する（C++ が正とする）
		struct Entry
		{
			const char* m_key;
			core::data::FileExtensionType m_type;
		};
		constexpr Entry ENTRIES[] = {
			{ EXT_TYPE_NAME_EXECUTABLE, core::data::FileExtensionType::Executable },
			{ EXT_TYPE_NAME_DOCUMENT, core::data::FileExtensionType::Document },
			{ EXT_TYPE_NAME_IMAGE, core::data::FileExtensionType::Image },
			{ EXT_TYPE_NAME_AUDIO, core::data::FileExtensionType::Audio },
			{ EXT_TYPE_NAME_SOURCE_CODE, core::data::FileExtensionType::SourceCode },
			{ EXT_TYPE_NAME_SHORTCUT, core::data::FileExtensionType::Shortcut },
			{ EXT_TYPE_NAME_VIDEO, core::data::FileExtensionType::Video },
			{ EXT_TYPE_NAME_ARCHIVE, core::data::FileExtensionType::Archive },
			{ EXT_TYPE_NAME_UNKNOWN, core::data::FileExtensionType::Unknown },
		};

		auto fmt = [](float v) -> std::string {
			if (v == static_cast<int>(v))
				return std::to_string(static_cast<int>(v));
			std::ostringstream oss{};
			oss << v;
			return oss.str();
		};

		// ボーナスの項目定義。JS側はここで渡す m_statId でアイコンと日本語名を引く。
		// 略称（m_label）は装備スロット行の狭い欄に出す短い表記に使う
		struct StatField
		{
			const char* m_statId;
			const char* m_label;
			float core::data::FileExtensionBonus::* m_member;
			float m_scale; // 会心率は確率なので%へ直してから見せる
		};
		constexpr StatField STAT_FIELDS[] = {
			{ "hp", "HP", &core::data::FileExtensionBonus::hp, 1.0f },
			{ "atk", "ATK", &core::data::FileExtensionBonus::atk, 1.0f },
			{ "def", "DEF", &core::data::FileExtensionBonus::def, 1.0f },
			{ "spd", "SPD", &core::data::FileExtensionBonus::spd, 1.0f },
			{ "rng", "Range", &core::data::FileExtensionBonus::attackRange, 1.0f },
			{ "crit", "CRIT", &core::data::FileExtensionBonus::criticalRate, PERCENT_SCALE },
			{ "bspd", "B.SPD", &core::data::FileExtensionBonus::projectileSpeed, 1.0f },
			{ "brng", "B.RNG", &core::data::FileExtensionBonus::projectileRange, 1.0f },
		};

		// 短い説明文（装備スロット行の「ボーナス」欄用）
		auto describe = [&](core::data::FileExtensionType t) -> std::string
		{
			const auto& b = m_resourceManager.getExtensionBonus(t);
			std::string result{};
			for (const auto& f : STAT_FIELDS)
			{
				const float value{ b.*(f.m_member) * f.m_scale };
				if (value == 0.0f)
					continue;
				if (!result.empty()) result += ' ';
				result += f.m_label;
				result += '+';
				result += fmt(value);
			}
			return result;
		};

		// 項目ごとの内訳（拡張子ボーナス一覧でアイコン付きに描くため）
		auto breakdown = [&](core::data::FileExtensionType t) -> nlohmann::json
		{
			const auto& b = m_resourceManager.getExtensionBonus(t);
			nlohmann::json list = nlohmann::json::array();
			for (const auto& f : STAT_FIELDS)
			{
				const float value{ b.*(f.m_member) * f.m_scale };
				if (value == 0.0f)
					continue;
				list.push_back({ { "stat", f.m_statId }, { "value", value } });
			}
			return list;
		};

		try
		{
			nlohmann::json resp;
			resp[platform::window::WindowConstants::JSON_KEY_TYPE]  = platform::window::WindowConstants::MESSAGE_TYPE_BONUS_INFO;
			resp[platform::window::WindowConstants::JSON_KEY_DESCRIPTIONS] = nlohmann::json::object();
			resp[platform::window::WindowConstants::JSON_KEY_EXTENSIONS] = nlohmann::json::object();
			resp[platform::window::WindowConstants::JSON_KEY_BONUS_STATS] = nlohmann::json::object();
			for (const auto& e : ENTRIES)
			{
				resp[platform::window::WindowConstants::JSON_KEY_DESCRIPTIONS][e.m_key] = describe(e.m_type);
				resp[platform::window::WindowConstants::JSON_KEY_BONUS_STATS][e.m_key] = breakdown(e.m_type);
				// 対象の拡張子も判定表から取り出して送る。
				// 「.exe など」と省略すると、どの拡張子が該当するのか確かめる手段が無くなる
				resp[platform::window::WindowConstants::JSON_KEY_EXTENSIONS][e.m_key] =
				    game::utility::FileExtensionTypeResolver::joinExtensions(e.m_type);
			}
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
