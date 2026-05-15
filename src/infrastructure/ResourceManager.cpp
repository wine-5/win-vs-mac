#include "ResourceManager.h"
#include <DxLib.h>
#include "core/interface/ILogger.h"
#include "thirdparty/nlohmann/json.hpp"
#include <fstream>
#include <cassert>
#include <stdexcept>
#include "constant/JsonKeys.h"

namespace infrastructure
{
	ResourceManager::ResourceManager(const core::iface::IStringConverter& stringConverter)
		: m_stringConverter{stringConverter}
	{
		std::ifstream file("assets/config/resources.json");
		throwIfFileNotOpen(file, "assets/config/resources.json");
		file.imbue(std::locale(".UTF8"));
		const nlohmann::json j = nlohmann::json::parse(file);

		// モデルリソースを読み込む
		auto resourceList{ loadResourceList(j) };
		for (const auto& res : resourceList)
		{
			auto metadata{ parseJsonFile(res.m_path) };
			m_metadata[res.m_id] = metadata;
		}

		// フォントを読み込む
		auto fontList{ loadFontList(j) };
		for (const auto& font : fontList)
		{
			AddFontResourceEx(font.m_path.c_str(), FR_PRIVATE, nullptr);
			m_fontNames[font.m_id] = font.m_name;
			m_fontPaths[font.m_id] = font.m_path;
		}

		loadJobsFromJson();
	}

	ResourceManager::~ResourceManager()
	{
		for (const auto& [id, path] : m_fontPaths)
			RemoveFontResourceEx(path.c_str(), FR_PRIVATE, nullptr);
	}

	int ResourceManager::loadModelById(const std::string_view modelId)
	{
		std::string modelIdStr(modelId);
		// メタデータを取得
		auto it{ m_metadata.find(modelIdStr) };
		if (it == m_metadata.end())
		{
			LOG_E("モデルID '%s' が見つかりません", modelIdStr.c_str());
			return -1;
		}

		const auto& metadata = it->second;

		// 既にロード済みか確認
		auto handleIt{ m_modelHandles.find(modelIdStr) };
		if (handleIt != m_modelHandles.end())
		{
			return handleIt->second;
		}

		// モデルをロード
		int handle{ MV1LoadModel(metadata.modelPath.c_str()) };
		if (handle == -1)
		{
			LOG_E("モデルの読み込みに失敗しました: %s", metadata.modelPath.c_str());
			return -1;
		}

		// スケールを適用
		VECTOR scale = VGet(metadata.scale.x, metadata.scale.y, metadata.scale.z);
		MV1SetScale(handle, scale);

		// colliderSizeが0の場合、モデルのAABBから自動計算
		if (metadata.colliderSize.x == 0.0f &&
			metadata.colliderSize.y == 0.0f &&
			metadata.colliderSize.z == 0.0f)
		{
			// 非constなメタデータを取得して更新
			auto& mutableMetadata = m_metadata[modelIdStr];

			VECTOR vMin = MV1GetFrameMinVertexLocalPosition(handle, -1);
			VECTOR vMax = MV1GetFrameMaxVertexLocalPosition(handle, -1);

			mutableMetadata.colliderSize.x = vMax.x - vMin.x;
			mutableMetadata.colliderSize.y = vMax.y - vMin.y;
			mutableMetadata.colliderSize.z = vMax.z - vMin.z;

			LOG("'%s' のコライダーサイズを自動計算: (%.2f, %.2f, %.2f)",
				modelIdStr.c_str(),
				mutableMetadata.colliderSize.x,
				mutableMetadata.colliderSize.y,
				mutableMetadata.colliderSize.z);
		}

		m_modelHandles[modelIdStr] = handle;
		return handle;
	}

	std::optional<core::data::ModelMetadata> ResourceManager::getMetadata(const std::string_view modelId) const
	{
		std::string modelIdStr(modelId);
		auto it{ m_metadata.find(modelIdStr) };
		if (it == m_metadata.end())
			return std::nullopt;

		return it->second;
	}

	std::optional<std::string> ResourceManager::getFontName(const std::string_view fontId) const
	{
		std::string key{ fontId };
		auto it{ m_fontNames.find(key) };
		if (it == m_fontNames.end())
			return std::nullopt;
		return it->second;
	}

	std::vector<ResourceManager::FontDataDefinition> ResourceManager::loadFontList(const nlohmann::json& json)
	{
		std::vector<FontDataDefinition> fonts;
		if (!json.contains("fonts"))
			return fonts;

		for (const auto& item : json["fonts"])
		{
			FontDataDefinition def;
			def.m_id = item["id"];
			def.m_path = item["path"];
			def.m_name = item["name"];
			fonts.push_back(def);
		}
		return fonts;
	}

	std::vector<ResourceManager::ResourceDefinition> ResourceManager::loadResourceList(const nlohmann::json& json)
	{
		std::vector<ResourceDefinition> resources;

		for (const auto& item : json["resources"])
		{
			ResourceDefinition def;
			def.m_id = item["id"];
			def.m_path = item["path"];
			resources.push_back(def);
		}

		return resources;
	}

	core::data::ModelMetadata ResourceManager::parseJsonFile(const std::string& filePath)
	{
		using namespace infrastructure::constant; // json_keysを使いやすく

		std::ifstream file(filePath);
		throwIfFileNotOpen(file, filePath);
		file.imbue(std::locale(".UTF8"));

		nlohmann::json j = nlohmann::json::parse(file);

		core::data::ModelMetadata metadata;
		metadata.id = j[json_keys::ID];
		metadata.category = j[json_keys::CATEGORY];
		metadata.modelPath = j[json_keys::MODEL][json_keys::PATH];

		// スケール
		metadata.scale.x = j[json_keys::MODEL][json_keys::SCALE][0];
		metadata.scale.y = j[json_keys::MODEL][json_keys::SCALE][1];
		metadata.scale.z = j[json_keys::MODEL][json_keys::SCALE][2];

		// コライダーサイズ
		metadata.colliderSize.x = j[json_keys::COLLIDER][json_keys::SIZE][0];
		metadata.colliderSize.y = j[json_keys::COLLIDER][json_keys::SIZE][1];
		metadata.colliderSize.z = j[json_keys::COLLIDER][json_keys::SIZE][2];

		// コライダーオフセット
		metadata.colliderOffset.x = j[json_keys::COLLIDER][json_keys::OFFSET][0];
		metadata.colliderOffset.y = j[json_keys::COLLIDER][json_keys::OFFSET][1];
		metadata.colliderOffset.z = j[json_keys::COLLIDER][json_keys::OFFSET][2];

		// Transform情報（直接メンバに代入、findコスト削減）
		if (j.contains(json_keys::TRANSFORM))
		{
			if (j[json_keys::TRANSFORM].contains("posX"))
				metadata.position.x = j[json_keys::TRANSFORM]["posX"];
			if (j[json_keys::TRANSFORM].contains("posY"))
				metadata.position.y = j[json_keys::TRANSFORM]["posY"];
			if (j[json_keys::TRANSFORM].contains("posZ"))
				metadata.position.z = j[json_keys::TRANSFORM]["posZ"];
			if (j[json_keys::TRANSFORM].contains("rotX"))
				metadata.rotation.x = j[json_keys::TRANSFORM]["rotX"];
			if (j[json_keys::TRANSFORM].contains("rotY"))
				metadata.rotation.y = j[json_keys::TRANSFORM]["rotY"];
			if (j[json_keys::TRANSFORM].contains("rotZ"))
				metadata.rotation.z = j[json_keys::TRANSFORM]["rotZ"];
		}

		// アニメーション（stringProperties）
		if (j.contains(json_keys::ANIMATIONS))
		{
			if (j[json_keys::ANIMATIONS].contains(json_keys::IDLE))
				metadata.stringProperties["idleAnim"] = j[json_keys::ANIMATIONS][json_keys::IDLE];
			if (j[json_keys::ANIMATIONS].contains(json_keys::WALK))
				metadata.stringProperties["walkAnim"] = j[json_keys::ANIMATIONS][json_keys::WALK];
		}

		// ゲームプレイパラメータ（floatProperties - Entity固有のレアなパラメータのみ）
		if (j.contains(json_keys::GAMEPLAY))
		{
			if (j[json_keys::GAMEPLAY].contains(json_keys::MOVE_SPEED))
				metadata.floatProperties["moveSpeed"] = j[json_keys::GAMEPLAY][json_keys::MOVE_SPEED];

			if (j[json_keys::GAMEPLAY].contains(json_keys::DETECTION_RANGE))
				metadata.floatProperties["detectionRange"] = j[json_keys::GAMEPLAY][json_keys::DETECTION_RANGE];
			if (j[json_keys::GAMEPLAY].contains(json_keys::ATTACK_RANGE))
				metadata.floatProperties["attackRange"] = j[json_keys::GAMEPLAY][json_keys::ATTACK_RANGE];
			if (j[json_keys::GAMEPLAY].contains(json_keys::MAX_HP))
				metadata.floatProperties["maxHp"] = j[json_keys::GAMEPLAY][json_keys::MAX_HP];
			if (j[json_keys::GAMEPLAY].contains(json_keys::DEFENCE))
				metadata.floatProperties["defence"] = j[json_keys::GAMEPLAY][json_keys::DEFENCE];
			if (j[json_keys::GAMEPLAY].contains(json_keys::ATTACK_POWER))
				metadata.floatProperties["attackPower"] = j[json_keys::GAMEPLAY][json_keys::ATTACK_POWER];
			if (j[json_keys::GAMEPLAY].contains(json_keys::ATTACK_COOLDOWN))
				metadata.floatProperties["attackCooldown"] = j[json_keys::GAMEPLAY][json_keys::ATTACK_COOLDOWN];
		}

		return metadata;
	}

	void ResourceManager::loadJobsFromJson()
	{
		std::ifstream file("assets/data/jobData.json");
		throwIfFileNotOpen(file, "assets/data/jobData.json");
		file.imbue(std::locale(".UTF8"));

		auto json = nlohmann::json::parse(file);
		const auto& jobs{ json["jobs"] };

		for (size_t i = 0; i < jobs.size() && i < m_jobTable.size(); ++i)
		{
			m_jobTable[i].m_id = jobs[i]["id"];
			m_jobTable[i].m_name = m_stringConverter.utf8ToShiftJis(jobs[i]["name"].get<std::string>());
			m_jobTable[i].m_skillName = m_stringConverter.utf8ToShiftJis(jobs[i]["skillName"].get<std::string>());
			m_jobTable[i].m_hp = jobs[i]["hp"];
			m_jobTable[i].m_atk = jobs[i]["atk"];
			m_jobTable[i].m_def = jobs[i]["def"];
			m_jobTable[i].m_spd = jobs[i]["spd"];
		}
	}

	int ResourceManager::getJobCount() const noexcept
	{
		return core::constant::JOB_COUNT;
	}

	core::iface::JobInfo ResourceManager::getJobInfo(core::constant::JobType jobType) const
	{
		return m_jobTable[static_cast<int>(jobType)];
	}

	void ResourceManager::throwIfFileNotOpen(const std::ifstream& file, const std::string& filePath)
	{
		if (file.is_open()) return;

		LOG_E("FATAL: ファイルを開けませんでした: %s", filePath.c_str());
		assert(false && "致命的エラー: ファイルが見つかりません。ファイルパスを確認してください。");
		throw std::runtime_error("ファイルを開けませんでした: " + filePath);
	}
}