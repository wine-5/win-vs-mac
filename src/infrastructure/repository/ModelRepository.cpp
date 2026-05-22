#include "ModelRepository.h"
#include <DxLib.h>
#include <fstream>
#include <stdexcept>
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"

namespace infrastructure
{
	ModelRepository::ModelRepository()
	{
		std::ifstream file("assets/config/resources.json");
		if (!file.is_open())
			throw std::runtime_error("assets/config/resources.json を開けませんでした");

		const nlohmann::json j = nlohmann::json::parse(file);
		for (const auto& res : loadResourceList(j))
			m_metadata[res.m_id] = parseJsonFile(res.m_path);
		for (const auto& res : loadRawModelList(j))
			m_rawModelPaths[res.m_id] = res.m_path;
	}

	int ModelRepository::loadModelById(std::string_view modelId)
	{
		std::string id(modelId);

		// raw modelの場合は直接MV1LoadModel
		{
			auto rawIt{ m_rawModelPaths.find(id) };
			if (rawIt != m_rawModelPaths.end())
			{
				auto handleIt{ m_modelHandles.find(id) };
				if (handleIt != m_modelHandles.end())
					return handleIt->second;

				int handle{ MV1LoadModel(rawIt->second.c_str()) };
				if (handle == -1)
					LOG_E("モデルの読み込みに失敗しました: %s", rawIt->second.c_str());
				else
					m_modelHandles[id] = handle;
				return handle;
			}
		}

		auto metaIt{ m_metadata.find(id) };
		if (metaIt == m_metadata.end())
		{
			LOG_E("モデルID '%s' が見つかりません", id.c_str());
			return -1;
		}

		auto handleIt{ m_modelHandles.find(id) };
		if (handleIt != m_modelHandles.end())
			return handleIt->second;

		const auto& metadata = metaIt->second;
		int handle{ MV1LoadModel(metadata.modelPath.c_str()) };
		if (handle == -1)
		{
			LOG_E("モデルの読み込みに失敗しました: %s", metadata.modelPath.c_str());
			return -1;
		}

		VECTOR scale = VGet(metadata.scale.x, metadata.scale.y, metadata.scale.z);
		MV1SetScale(handle, scale);

		if (metadata.colliderSize.x == 0.0f &&
			metadata.colliderSize.y == 0.0f &&
			metadata.colliderSize.z == 0.0f)
		{
			auto& mutableMeta = m_metadata[id];
			VECTOR vMin = MV1GetFrameMinVertexLocalPosition(handle, -1);
			VECTOR vMax = MV1GetFrameMaxVertexLocalPosition(handle, -1);
			mutableMeta.colliderSize.x = vMax.x - vMin.x;
			mutableMeta.colliderSize.y = vMax.y - vMin.y;
			mutableMeta.colliderSize.z = vMax.z - vMin.z;

			LOG("'%s' のコライダーサイズを自動計算: (%.2f, %.2f, %.2f)",
				id.c_str(),
				mutableMeta.colliderSize.x,
				mutableMeta.colliderSize.y,
				mutableMeta.colliderSize.z);
		}

		m_modelHandles[id] = handle;
		return handle;
	}

	std::optional<core::data::ModelMetadata> ModelRepository::getMetadata(std::string_view modelId) const
	{
		auto it{ m_metadata.find(std::string(modelId)) };
		if (it == m_metadata.end())
			return std::nullopt;
		return it->second;
	}

	std::vector<ModelRepository::ResourceDefinition> ModelRepository::loadResourceList(const nlohmann::json& json)
	{
		std::vector<ResourceDefinition> resources;
		if (!json.contains("resources"))
			return resources;

		for (const auto& item : json["resources"])
			resources.push_back({ item["id"], item["path"] });
		return resources;
	}

	std::vector<ModelRepository::ResourceDefinition> ModelRepository::loadRawModelList(const nlohmann::json& json)
	{
		std::vector<ResourceDefinition> resources;
		if (!json.contains("rawModels"))
			return resources;

		for (const auto& item : json["rawModels"])
			resources.push_back({ item["id"], item["path"] });
		return resources;
	}

	core::data::ModelMetadata ModelRepository::parseJsonFile(const std::string& filePath)
	{
		std::ifstream file(filePath);
		if (!file.is_open())
			throw std::runtime_error("ファイルを開けませんでした: " + filePath);

		nlohmann::json j = nlohmann::json::parse(file);
		core::data::ModelMetadata metadata;
		metadata.id        = j["id"];
		metadata.category  = j["category"];
		metadata.modelPath = j["model"]["path"];
		metadata.scale.x   = j["model"]["scale"][0];
		metadata.scale.y   = j["model"]["scale"][1];
		metadata.scale.z   = j["model"]["scale"][2];
		metadata.colliderSize.x   = j["collider"]["size"][0];
		metadata.colliderSize.y   = j["collider"]["size"][1];
		metadata.colliderSize.z   = j["collider"]["size"][2];
		metadata.colliderOffset.x = j["collider"]["offset"][0];
		metadata.colliderOffset.y = j["collider"]["offset"][1];
		metadata.colliderOffset.z = j["collider"]["offset"][2];

		if (j.contains("transform"))
		{
			auto& tf = j["transform"];
			if (tf.contains("posX")) metadata.position.x = tf["posX"];
			if (tf.contains("posY")) metadata.position.y = tf["posY"];
			if (tf.contains("posZ")) metadata.position.z = tf["posZ"];
			if (tf.contains("rotX")) metadata.rotation.x = tf["rotX"];
			if (tf.contains("rotY")) metadata.rotation.y = tf["rotY"];
			if (tf.contains("rotZ")) metadata.rotation.z = tf["rotZ"];
		}

		if (j.contains("animations"))
		{
			auto& anim = j["animations"];
			if (anim.contains("idle")) metadata.stringProperties["idleAnim"] = anim["idle"];
			if (anim.contains("walk")) metadata.stringProperties["walkAnim"] = anim["walk"];
		}

		if (j.contains("gameplay"))
		{
			auto& gp = j["gameplay"];
			if (gp.contains("moveSpeed"))      metadata.floatProperties["moveSpeed"]      = gp["moveSpeed"];
			if (gp.contains("detectionRange")) metadata.floatProperties["detectionRange"] = gp["detectionRange"];
			if (gp.contains("attackRange"))    metadata.floatProperties["attackRange"]    = gp["attackRange"];
			if (gp.contains("maxHp"))          metadata.floatProperties["maxHp"]          = gp["maxHp"];
			if (gp.contains("defence"))        metadata.floatProperties["defence"]        = gp["defence"];
			if (gp.contains("attackPower"))    metadata.floatProperties["attackPower"]    = gp["attackPower"];
			if (gp.contains("attackCooldown")) metadata.floatProperties["attackCooldown"] = gp["attackCooldown"];
		}

		return metadata;
	}
}