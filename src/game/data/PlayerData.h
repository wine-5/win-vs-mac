#pragma once
#include <string>
#include "core/Vector3.h"
#include "core/data/ModelMetadata.h"

namespace game::data
{
    /**
     * @brief Playerのデータを保持するクラス
     */
    class PlayerData
    {
    public:
        /**
         * @brief ModelMetadataからPlayerDataを生成
         * @param metadata ResourceManagerから取得したメタデータ
         * @return PlayerDataインスタンス
         */
        static PlayerData fromMetadata(const core::data::ModelMetadata& metadata)
        {
            PlayerData data;
            data.m_modelPath = metadata.modelPath;
            data.m_colliderSize = metadata.colliderSize;
            data.m_colliderOffset = metadata.colliderOffset;

            // アニメーションパス（stringProperties から取得）
            auto idleIt = metadata.stringProperties.find("idleAnim");
            if (idleIt != metadata.stringProperties.end())
                data.m_idleAnimPath = idleIt->second;

            auto walkIt = metadata.stringProperties.find("walkAnim");
            if (walkIt != metadata.stringProperties.end())
                data.m_walkAnimPath = walkIt->second;

            // moveSpeed（floatProperties から取得）
            auto moveSpeedIt = metadata.floatProperties.find("moveSpeed");
            if (moveSpeedIt != metadata.floatProperties.end())
                data.m_moveSpeed = moveSpeedIt->second;

            return data;
        }

        const std::string& getModelPath()    const { return m_modelPath; }
        const std::string& getIdleAnimPath() const { return m_idleAnimPath; }
        const std::string& getWalkAnimPath() const { return m_walkAnimPath; }
        float              getMoveSpeed()    const { return m_moveSpeed; }
        core::Vector3      getColliderSize() const { return m_colliderSize; }
        core::Vector3      getColliderOffset() const { return m_colliderOffset; }

    private:
        // JSON読み込み失敗時はFail Fast
        std::string   m_modelPath;
        std::string   m_idleAnimPath;
        std::string   m_walkAnimPath;
        float         m_moveSpeed;
        core::Vector3 m_colliderSize;
        core::Vector3 m_colliderOffset;
    };
}