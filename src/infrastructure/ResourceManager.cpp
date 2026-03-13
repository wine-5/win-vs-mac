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
    ResourceManager::ResourceManager()
    {
        // resources.jsonから全リソースを読み込む
        auto resourceList = loadResourceList("assets/config/resources.json");
        for (const auto& res : resourceList)
        {
            auto metadata = parseJsonFile(res.m_path);
            m_metadata[res.m_id] = metadata;
        }
    }

    int ResourceManager::loadModelById(const std::string& modelId)
    {
        // メタデータを取得
        auto it = m_metadata.find(modelId);
        if (it == m_metadata.end())
        {
            LOG_E("モデルID '%s' が見つかりません", modelId.c_str());
            return -1;
        }

        const auto& metadata = it->second;

        // 既にロード済みか確認
        auto handleIt = m_modelHandles.find(modelId);
        if (handleIt != m_modelHandles.end())
        {
            return handleIt->second;
        }

        // モデルをロード
        int handle = MV1LoadModel(metadata.modelPath.c_str());
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
            auto& mutableMetadata = m_metadata[modelId];

            VECTOR vMin = MV1GetFrameMinVertexLocalPosition(handle, -1);
            VECTOR vMax = MV1GetFrameMaxVertexLocalPosition(handle, -1);

            mutableMetadata.colliderSize.x = vMax.x - vMin.x;
            mutableMetadata.colliderSize.y = vMax.y - vMin.y;
            mutableMetadata.colliderSize.z = vMax.z - vMin.z;

            LOG("'%s' のコライダーサイズを自動計算: (%.2f, %.2f, %.2f)",
                modelId.c_str(),
                mutableMetadata.colliderSize.x,
                mutableMetadata.colliderSize.y,
                mutableMetadata.colliderSize.z);
        }

        m_modelHandles[modelId] = handle;
        return handle;
    }

    std::optional<core::data::ModelMetadata> ResourceManager::getMetadata(const std::string& modelId) const
    {
        auto it = m_metadata.find(modelId);
        if (it == m_metadata.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    std::vector<ResourceManager::ResourceDefinition> ResourceManager::loadResourceList(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            LOG_E("FATAL: resources.jsonを開けませんでした: %s", filePath.c_str());
            assert(false && "致命的エラー: resources.jsonが見つかりません。ファイルパスを確認してください。");
            throw std::runtime_error("resources.jsonを開けませんでした: " + filePath);
        }

        nlohmann::json j = nlohmann::json::parse(file);
        std::vector<ResourceDefinition> resources;

        for (const auto& item : j["resources"])
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
        using namespace infrastructure::constant;  // json_keysを使いやすく

        std::ifstream file(filePath);
        if (!file.is_open())
        {
            LOG_E("FATAL: JSONファイルを開けませんでした: %s", filePath.c_str());
            assert(false && "致命的エラー: JSONファイルが見つかりません。ファイルパスを確認してください。");
            throw std::runtime_error("JSONファイルを開けませんでした: " + filePath);
        }

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
        }

        return metadata;
    }
}