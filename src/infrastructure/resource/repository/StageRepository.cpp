#include "StageRepository.h"
#include <fstream>
#include <stdexcept>
#include "thirdparty/nlohmann/json.hpp"

namespace
{
	/**
	 * @brief JSONのspawn要素をSpawnMetadataへ変換する
	 * @param j spawn要素のJSONオブジェクト
	 * @return 変換したSpawnMetadata
	 */
	core::data::SpawnMetadata parseSpawn(const nlohmann::json& j)
	{
		core::data::SpawnMetadata spawn{};
		spawn.m_type = j["type"].get<std::string>();
		spawn.m_position.x = j["position"][0];
		spawn.m_position.y = j["position"][1];
		spawn.m_position.z = j["position"][2];
		return spawn;
	}
} // namespace

namespace infrastructure::resource::repository
{
	StageRepository::StageRepository()
	{
		std::ifstream file("assets/data/stageData.json");
		if (!file.is_open())
			throw std::runtime_error("assets/data/stageData.jsonを開けませんでした");

		const nlohmann::json j = nlohmann::json::parse(file);
		for (const auto& spawn : j["spawns"])
			m_stageMetadata.m_spawns.push_back(parseSpawn(spawn));

		// DEBUG: デバッグ用に contains() でチェック。
		// リリース時は必須項目のため直接 j["mac"] を参照する。
		if (j.contains("mac"))
			m_stageMetadata.m_mac = parseSpawn(j["mac"]);
	}

	const core::data::StageMetadata& StageRepository::getStageMetadata() const noexcept
	{
		return m_stageMetadata;
	}
} // namespace infrastructure::resource::repository