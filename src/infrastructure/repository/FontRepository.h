#pragma once
#include <string>
#include <string_view>
#include <optional>
#include <unordered_map>
#include <vector>
#include "thirdparty/nlohmann/json.hpp"

namespace infrastructure
{
	class FontRepository
	{
	public:
		FontRepository();
		~FontRepository();

		std::optional<std::string> getFontName(std::string_view fontId) const;

	private:
		struct FontDefinition
		{
			std::string m_id;
			std::string m_path;
			std::string m_name;
		};

		std::vector<FontDefinition> loadFontList(const nlohmann::json& json);

		std::unordered_map<std::string, std::string> m_fontNames;
		std::unordered_map<std::string, std::string> m_fontPaths;
	};
}
