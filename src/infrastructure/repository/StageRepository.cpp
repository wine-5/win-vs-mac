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

namespace infrastructure::repository
{
	StageRepository::StageRepository()
	{
		std::ifstream file("assets/data/stageData.json");
		if (!file.is_open())
			throw std::runtime_error("assets/data/stageData.jsonを開けませんでした");

		const nlohmann::json j = nlohmann::json::parse(file);
		for(const auto& spawn : j["spawns"])
			m_stageMetadata.m_spawns.push_back(parseSpawn(spawn));

		m_stageMetadata.m_boss = parseSpawn(j["boss"]);
	}

	const core::data::StageMetadata& StageRepository::getStageMetadata() const noexcept
	{
		return m_stageMetadata;
	}
} // namespace infrastructure::repository