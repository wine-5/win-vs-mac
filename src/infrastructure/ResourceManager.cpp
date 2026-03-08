#include "ResourceManager.h"
#include <DxLib.h>
#include "core/interface/ILogger.h"
#include "thirdparty/nlohmann/json.hpp"
#include <fstream>
#include "constant/JsonKeys.h"

namespace infrastructure
{
    ResourceManager::ResourceManager()
    {
        // 起動時にplayer.jsonを読み込む
        loadPlayerData();
    }

    int ResourceManager::loadModel(const std::string& filePath)
    {
        // キャッシュにある場合は重複しないように
        auto it = m_modelCache.find(filePath);
        if (it != m_modelCache.end()) return it->second;

        int handle = MV1LoadModel(filePath.c_str());
        if (handle == -1)
        {
            LOG_E("モデルの読み込みに失敗しました: %s", filePath.c_str());
            return -1;
        }

        m_modelCache[filePath] = handle;
        return handle;
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

            LOG("Auto-calculated collider size for '%s': (%.2f, %.2f, %.2f)",
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

    void ResourceManager::loadPlayerData()
    {
        auto metadata = parseJsonFile("assets/data/playerData.json");
        m_metadata[metadata.id] = metadata;
        printfDx("Loaded player metadata: %s\n", metadata.id.c_str());
    }

    core::data::ModelMetadata ResourceManager::parseJsonFile(const std::string& filePath)
    {
        using namespace infrastructure::constant;  // JsonKeysを使いやすく

        std::ifstream file(filePath);
        if (!file.is_open())
        {
            LOG_E("JSONファイルを開けませんでした: %s", filePath.c_str());
            return {};
        }

        nlohmann::json j = nlohmann::json::parse(file);

        core::data::ModelMetadata metadata;
        metadata.id = j[JsonKeys::ID];
        metadata.category = j[JsonKeys::CATEGORY];
        metadata.modelPath = j[JsonKeys::MODEL][JsonKeys::PATH];

        // スケール
        metadata.scale.x = j[JsonKeys::MODEL][JsonKeys::SCALE][0];
        metadata.scale.y = j[JsonKeys::MODEL][JsonKeys::SCALE][1];
        metadata.scale.z = j[JsonKeys::MODEL][JsonKeys::SCALE][2];

        // コライダーサイズ
        metadata.colliderSize.x = j[JsonKeys::COLLIDER][JsonKeys::SIZE][0];
        metadata.colliderSize.y = j[JsonKeys::COLLIDER][JsonKeys::SIZE][1];
        metadata.colliderSize.z = j[JsonKeys::COLLIDER][JsonKeys::SIZE][2];

        // コライダーオフセット
        metadata.colliderOffset.x = j[JsonKeys::COLLIDER][JsonKeys::OFFSET][0];
        metadata.colliderOffset.y = j[JsonKeys::COLLIDER][JsonKeys::OFFSET][1];
        metadata.colliderOffset.z = j[JsonKeys::COLLIDER][JsonKeys::OFFSET][2];

        // アニメーション（stringProperties）
        if (j.contains(JsonKeys::ANIMATIONS))
        {
            if (j[JsonKeys::ANIMATIONS].contains(JsonKeys::IDLE))
                metadata.stringProperties["idleAnim"] = j[JsonKeys::ANIMATIONS][JsonKeys::IDLE];
            if (j[JsonKeys::ANIMATIONS].contains(JsonKeys::WALK))
                metadata.stringProperties["walkAnim"] = j[JsonKeys::ANIMATIONS][JsonKeys::WALK];
        }

        // ゲームプレイパラメータ（floatProperties）
        if (j.contains(JsonKeys::GAMEPLAY))
        {
            if (j[JsonKeys::GAMEPLAY].contains(JsonKeys::MOVE_SPEED))
                metadata.floatProperties["moveSpeed"] = j[JsonKeys::GAMEPLAY][JsonKeys::MOVE_SPEED];
        }

        return metadata;
    }
}