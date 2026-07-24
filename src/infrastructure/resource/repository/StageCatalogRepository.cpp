#include "StageCatalogRepository.h"
#include <fstream>
#include <stdexcept>
#include "thirdparty/nlohmann/json.hpp"

namespace
{
	/**
	 * @brief JSONのprops要素をPropDefinitionへ変換する
	 * @param j props要素のJSONオブジェクト
	 * @return 変換したPropDefinition
	 */
	core::data::PropDefinition parseProp(const nlohmann::json& j)
	{
		core::data::PropDefinition def{};
		def.m_id = j["id"].get<std::string>();
		def.m_modelPath = j["model"].get<std::string>();
		def.m_baseSize.x = j["baseSize"][0];
		def.m_baseSize.y = j["baseSize"][1];
		def.m_baseSize.z = j["baseSize"][2];
		def.m_collider = j["collider"].get<std::string>();
		if (j.contains("textureTile"))
			def.m_textureTile = j["textureTile"];
		return def;
	}
} // namespace

namespace infrastructure::resource::repository
{
	StageCatalogRepository::StageCatalogRepository()
	{
		std::ifstream file("assets/data/stageCatalog.json");
		if (!file.is_open())
			throw std::runtime_error("assets/data/stageCatalog.jsonを開けませんでした");

		const nlohmann::json j = nlohmann::json::parse(file);
		for (const auto& prop : j["props"])
		{
			core::data::PropDefinition def{ parseProp(prop) };
			m_props[def.m_id] = def;
		}
	}

	const core::data::PropDefinition& StageCatalogRepository::getProp(std::string_view type) const
	{
		auto it{ m_props.find(std::string(type)) };
		if (it == m_props.end())
			throw std::runtime_error("配置物の種類 '" + std::string(type) + "' がstageCatalog.jsonに存在しません");
		return it->second;
	}
} // namespace infrastructure::resource::repository
