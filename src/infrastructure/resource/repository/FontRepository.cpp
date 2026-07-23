#include "FontRepository.h"
#include <DxLib.h>
#include <stdexcept>

namespace infrastructure::resource::repository
{
	FontRepository::FontRepository(const nlohmann::json& j)
	{
		for (const auto& font : loadFontList(j))
		{
			AddFontResourceEx(font.m_path.c_str(), FR_PRIVATE, nullptr);
			m_fontNames[font.m_id] = font.m_name;
			m_fontPaths[font.m_id] = font.m_path;
		}
	}

	FontRepository::~FontRepository()
	{
		for (const auto& [id, path] : m_fontPaths)
			RemoveFontResourceEx(path.c_str(), FR_PRIVATE, nullptr);
	}

	std::optional<std::string> FontRepository::getFontName(std::string_view fontId) const
	{
		auto it{ m_fontNames.find(std::string(fontId)) };
		if (it == m_fontNames.end())
			return std::nullopt;
		return it->second;
	}

	std::vector<FontRepository::FontDefinition> FontRepository::loadFontList(const nlohmann::json& json)
	{
		std::vector<FontDefinition> fonts;
		if (!json.contains("fonts"))
			return fonts;

		for (const auto& item : json["fonts"])
			fonts.push_back({ item["id"], item["path"], item["name"] });
		return fonts;
	}
} // namespace infrastructure::resource::repository