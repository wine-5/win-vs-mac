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

	/**
	 * @brief JSONのprop要素をPropMetadataへ変換する
	 * @param j prop要素のJSONオブジェクト
	 * @return 変換したPropMetadata
	 */
	core::data::PropMetadata parseProp(const nlohmann::json& j)
	{
		core::data::PropMetadata prop{};
		prop.m_type = j["type"].get<std::string>();
		prop.m_position.x = j["position"][0];
		prop.m_position.y = j["position"][1];
		prop.m_position.z = j["position"][2];
		prop.m_rotation.x = j["rotation"][0];
		prop.m_rotation.y = j["rotation"][1];
		prop.m_rotation.z = j["rotation"][2];
		prop.m_size.x = j["size"][0];
		prop.m_size.y = j["size"][1];
		prop.m_size.z = j["size"][2];
		return prop;
	}

	/**
	 * @brief JSONのplayerStart要素をPlayerStartMetadataへ変換する
	 * @param j playerStart要素のJSONオブジェクト
	 * @return 変換したPlayerStartMetadata
	 */
	core::data::PlayerStartMetadata parsePlayerStart(const nlohmann::json& j)
	{
		core::data::PlayerStartMetadata start{};
		start.m_position.x = j["position"][0];
		start.m_position.y = j["position"][1];
		start.m_position.z = j["position"][2];
		start.m_rotationY = j["rotationY"];
		return start;
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

		// DEBUG: 移行期のため contains() で存在チェックする。
		// props[]・playerStart を持つ新スキーマへ全面移行したら必須項目として直接参照する。
		if (j.contains("playerStart"))
			m_stageMetadata.m_playerStart = parsePlayerStart(j["playerStart"]);

		if (j.contains("props"))
			for (const auto& prop : j["props"])
				m_stageMetadata.m_props.push_back(parseProp(prop));

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