#include "ExtensionBonusRepository.h"
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include "thirdparty/nlohmann/json.hpp"

namespace
{
	constexpr const char* BONUS_CONFIG_PATH{ "assets/data/extensionBonus.json" };

	// JSONのキー名と FileExtensionType の対応。enumの並びと1対1で保つこと
	constexpr std::pair<std::string_view, core::data::FileExtensionType> TYPE_KEYS[]{
		{ "executable", core::data::FileExtensionType::Executable },
		{ "document", core::data::FileExtensionType::Document },
		{ "image", core::data::FileExtensionType::Image },
		{ "audio", core::data::FileExtensionType::Audio },
		{ "archive", core::data::FileExtensionType::Archive },
		{ "unknown", core::data::FileExtensionType::Unknown },
	};
} // namespace

namespace infrastructure::resource::repository
{
	ExtensionBonusRepository::ExtensionBonusRepository()
	{
		std::ifstream file{ BONUS_CONFIG_PATH };
		if (!file.is_open())
			throw std::runtime_error{ std::string{ BONUS_CONFIG_PATH } + " を開けませんでした" };

		const nlohmann::json j{ nlohmann::json::parse(file) };
		if (!j.contains("bonuses"))
			throw std::runtime_error{ std::string{ BONUS_CONFIG_PATH } + ": 必須キー 'bonuses' がありません" };

		const auto& bonuses = j["bonuses"];
		for (const auto& [key, type] : TYPE_KEYS)
		{
			const std::string name{ key };
			if (!bonuses.contains(name))
				continue;

			const auto& entry = bonuses[name];
			auto& bonus = m_bonuses[static_cast<std::size_t>(type)];

			// 未設定の項目は0のまま（そのステータスには影響しない）
			const auto read{ [&entry](const char* field, float& out)
				{
				    if (entry.contains(field))
					    out = entry[field];
				} };
			read("atk", bonus.atk);
			read("spd", bonus.spd);
			read("def", bonus.def);
			read("hp", bonus.hp);
			read("attackRange", bonus.attackRange);
		}
	}

	const core::data::FileExtensionBonus& ExtensionBonusRepository::getBonus(
	    core::data::FileExtensionType type) const noexcept
	{
		return m_bonuses[static_cast<std::size_t>(type)];
	}
} // namespace infrastructure::resource::repository
