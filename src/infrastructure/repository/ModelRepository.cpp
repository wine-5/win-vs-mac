#include "ModelRepository.h"
#include <DxLib.h>
#include <fstream>
#include <stdexcept>
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"

namespace
{
	// 腕を広げたポーズだと横幅が実際の胴体より大きく出るため、水平方向を絞って胴体に沿わせる係数
	constexpr float HORIZONTAL_SHRINK{ 0.5f };
} // namespace

namespace infrastructure::repository
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
					LOG_E("モデルの読み込みに失敗しました: {}", rawIt->second.c_str());
				else
					m_modelHandles[id] = handle;
				return handle;
			}
		}

		auto metaIt{ m_metadata.find(id) };
		if (metaIt == m_metadata.end())
		{
			LOG_E("モデルID '{}' が見つかりません", id.c_str());
			return -1;
		}

		auto handleIt{ m_modelHandles.find(id) };
		if (handleIt != m_modelHandles.end())
			return handleIt->second;

		const auto& metadata = metaIt->second;
		int handle{ MV1LoadModel(metadata.modelPath.c_str()) };
		if (handle == -1)
		{
			LOG_E("モデルの読み込みに失敗しました: {}", metadata.modelPath.c_str());
			return -1;
		}

		VECTOR scale = VGet(metadata.scale.x, metadata.scale.y, metadata.scale.z);
		MV1SetScale(handle, scale);

		if (metadata.colliderSize.x == 0.0f &&
			metadata.colliderSize.y == 0.0f &&
			metadata.colliderSize.z == 0.0f)
		{
			auto& mutableMeta = m_metadata[id];

			// モデル全体の頂点からAABBを求める（参照メッシュはスケール適用後のワールド座標で取得する）
			MV1SetupReferenceMesh(handle, -1, TRUE);
			const MV1_REF_POLYGONLIST refPoly{ MV1GetReferenceMesh(handle, -1, TRUE) };

			if (refPoly.VertexNum > 0)
			{
				VECTOR vMin{ refPoly.Vertexs[0].Position };
				VECTOR vMax{ refPoly.Vertexs[0].Position };
				for (int i{ 1 }; i < refPoly.VertexNum; ++i)
				{
					const VECTOR& p{ refPoly.Vertexs[i].Position };
					vMin.x = (p.x < vMin.x) ? p.x : vMin.x;
					vMin.y = (p.y < vMin.y) ? p.y : vMin.y;
					vMin.z = (p.z < vMin.z) ? p.z : vMin.z;
					vMax.x = (p.x > vMax.x) ? p.x : vMax.x;
					vMax.y = (p.y > vMax.y) ? p.y : vMax.y;
					vMax.z = (p.z > vMax.z) ? p.z : vMax.z;
				}

				// 地面などのステージモデルは実寸のまま、キャラは腕幅を絞るため水平方向を縮小する
				const float horizontalScale{ (mutableMeta.category == "stage") ? 1.0f : HORIZONTAL_SHRINK };

				// 参照メッシュはスケール適用済みなのでそのままワールド寸法になる
				mutableMeta.colliderSize.x = (vMax.x - vMin.x) * horizontalScale;
				mutableMeta.colliderSize.y = vMax.y - vMin.y;
				mutableMeta.colliderSize.z = (vMax.z - vMin.z) * horizontalScale;

				// コライダー中心も自動計算する（足元原点モデルなら高さの半分が中心になる）
				mutableMeta.colliderOffset.x = (vMax.x + vMin.x) * 0.5f;
				mutableMeta.colliderOffset.y = (vMax.y + vMin.y) * 0.5f;
				mutableMeta.colliderOffset.z = (vMax.z + vMin.z) * 0.5f;

			}
			else
				LOG_E("'{}' のコライダー自動計算に失敗しました（頂点が取得できません）", id.c_str());

			MV1TerminateReferenceMesh(handle, -1, TRUE);
		}

		m_modelHandles[id] = handle;
		return handle;
	}

	int ModelRepository::duplicateModel(int modelHandle)
	{
		if (modelHandle == -1)
			return -1;

		int duplicated{ MV1DuplicateModel(modelHandle) };
		if (duplicated == -1)
			LOG_E("モデルハンドルの複製に失敗しました: {}", modelHandle);
		return duplicated;
	}

	float ModelRepository::computeBoundingRadius(int modelHandle, float scale) const
	{
		if (modelHandle == -1)
			return 0.0f;

		// 参照メッシュからAABBを求める（水平方向の大きい辺の半分を半径とする）
		MV1SetupReferenceMesh(modelHandle, -1, TRUE);
		const MV1_REF_POLYGONLIST refPoly{ MV1GetReferenceMesh(modelHandle, -1, TRUE) };

		float radius{ 0.0f };
		if (refPoly.VertexNum > 0)
		{
			VECTOR vMin{ refPoly.Vertexs[0].Position };
			VECTOR vMax{ refPoly.Vertexs[0].Position };
			for (int i{ 1 }; i < refPoly.VertexNum; ++i)
			{
				const VECTOR& p{ refPoly.Vertexs[i].Position };
				vMin.x = (p.x < vMin.x) ? p.x : vMin.x;
				vMin.z = (p.z < vMin.z) ? p.z : vMin.z;
				vMax.x = (p.x > vMax.x) ? p.x : vMax.x;
				vMax.z = (p.z > vMax.z) ? p.z : vMax.z;
			}
			const float sizeX{ vMax.x - vMin.x };
			const float sizeZ{ vMax.z - vMin.z };
			radius = 0.5f * ((sizeX > sizeZ) ? sizeX : sizeZ) * scale;
		}

		MV1TerminateReferenceMesh(modelHandle, -1, TRUE);
		return radius;
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
			if (gp.contains("dashMultiplier"))
				metadata.floatProperties["dashMultiplier"] = gp["dashMultiplier"];
			if (gp.contains("detectionRange")) metadata.floatProperties["detectionRange"] = gp["detectionRange"];
			if (gp.contains("attackRange"))    metadata.floatProperties["attackRange"]    = gp["attackRange"];
			if (gp.contains("maxHp"))          metadata.floatProperties["maxHp"]          = gp["maxHp"];
			if (gp.contains("defence"))        metadata.floatProperties["defence"]        = gp["defence"];
			if (gp.contains("attackPower"))    metadata.floatProperties["attackPower"]    = gp["attackPower"];
			if (gp.contains("attackCooldown")) metadata.floatProperties["attackCooldown"] = gp["attackCooldown"];
			if (gp.contains("hoverHeight"))
				metadata.floatProperties["hoverHeight"] = gp["hoverHeight"];
			if (gp.contains("preferredDistanceMin"))
				metadata.floatProperties["preferredDistanceMin"] = gp["preferredDistanceMin"];
			if (gp.contains("preferredDistanceMax"))
				metadata.floatProperties["preferredDistanceMax"] = gp["preferredDistanceMax"];
			if (gp.contains("fireCooldown"))
				metadata.floatProperties["fireCooldown"] = gp["fireCooldown"];
			if (gp.contains("facingYawOffset"))
				metadata.floatProperties["facingYawOffset"] = gp["facingYawOffset"];
		}

		return metadata;
	}
} // namespace infrastructure::repository