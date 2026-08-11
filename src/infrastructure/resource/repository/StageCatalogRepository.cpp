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
		if (j.contains("role"))
			def.m_role = j["role"].get<std::string>();
		if (j.contains("textureTile"))
			def.m_textureTile = j["textureTile"];
		if (j.contains("scrollU"))
			def.m_scrollU = j["scrollU"];
		if (j.contains("scrollV"))
			def.m_scrollV = j["scrollV"];

		// バランス調整で触る値は tuning にまとめてある。
		// モデルパスや大きさと同じ並びに混ぜると、数値を調整したいときに
		// 素材の定義を1件ずつ読み飛ばすことになる
		if (!j.contains("tuning"))
			return def;

		const auto& tuning{ j["tuning"] };
		if (tuning.contains("hitsToBreak"))
			def.m_hitsToBreak = tuning["hitsToBreak"];
		if (tuning.contains("dropExtension"))
			def.m_dropExtension = tuning["dropExtension"].get<std::string>();
		if (tuning.contains("dropCount"))
			def.m_dropCount = tuning["dropCount"];
		if (tuning.contains("grantsEquipSlot"))
			def.m_grantsEquipSlot = tuning["grantsEquipSlot"];
		if (tuning.contains("spawnEnemyType"))
			def.m_spawnEnemyType = tuning["spawnEnemyType"].get<std::string>();
		if (tuning.contains("spawnEnemyCount"))
			def.m_spawnEnemyCount = tuning["spawnEnemyCount"];
		if (tuning.contains("jackpotChance"))
			def.m_jackpotChance = tuning["jackpotChance"];
		if (tuning.contains("jackpotStatMultiplier"))
			def.m_jackpotStatMultiplier = tuning["jackpotStatMultiplier"];
		if (tuning.contains("slideAccel"))
			def.m_slideAccel = tuning["slideAccel"];
		if (tuning.contains("conveyorSpeed"))
			def.m_conveyorSpeed = tuning["conveyorSpeed"];
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

		loadBlockTable();
	}

	void StageCatalogRepository::loadBlockTable()
	{
		std::ifstream file("assets/data/stageBalance.json");
		if (!file.is_open())
			throw std::runtime_error("assets/data/stageBalance.jsonを開けませんでした");

		const nlohmann::json j = nlohmann::json::parse(file);

		// 抽選表そのものは無くても動く（配置がすべて具体的な種類なら不要）
		if (!j.contains("blockTable") || !j["blockTable"].contains("entries"))
			return;

		for (const auto& entry : j["blockTable"]["entries"])
		{
			// カタログを跨いだ突き合わせ。ここで止めないと、idを書き間違えた種類が
			// 「抽選には入っているが実体が無い」まま起動し、出現しない理由が分からなくなる
			const auto type{ entry["type"].get<std::string>() };
			if (m_props.find(type) == m_props.end())
				throw std::runtime_error("stageBalance.jsonのblockTableの '" + type + "' がstageCatalog.jsonのpropsに存在しません");

			m_blockTable.m_entries.push_back({ type, entry["weight"].get<float>() });
		}
	}

	const core::data::BlockTable& StageCatalogRepository::getBlockTable() const noexcept
	{
		return m_blockTable;
	}

	const core::data::PropDefinition& StageCatalogRepository::getProp(std::string_view type) const
	{
		auto it{ m_props.find(std::string(type)) };
		if (it == m_props.end())
			throw std::runtime_error("配置物の種類 '" + std::string(type) + "' がstageCatalog.jsonに存在しません");
		return it->second;
	}
} // namespace infrastructure::resource::repository
