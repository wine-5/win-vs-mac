#pragma once
#include <string>
#include "core/Vector3.h"
#include "core/data/ModelMetadata.h"

namespace game::data
{
    /**
     * @brief Groundのデータを保持するクラス
     */
    class GroundData
    {
    public:
        /**
         * @brief ModelMetadataからGroundDataを生成
         * @param metadata ResourceManagerから取得したメタデータ
         * @return GroundDataインスタンス
         */
        static GroundData fromMetadata(const core::data::ModelMetadata& metadata)
        {
            GroundData data;
            data.m_modelPath = metadata.modelPath;
            data.m_scale = metadata.scale;
            data.m_position = metadata.position;
            data.m_rotation = metadata.rotation;
            data.m_colliderSize = metadata.colliderSize;
            data.m_colliderOffset = metadata.colliderOffset;

            return data;
        }

        const std::string& getModelPath()      const { return m_modelPath; }
        core::Vector3      getPosition()       const { return m_position; }
        core::Vector3      getRotation()       const { return m_rotation; }
        core::Vector3      getScale()          const { return m_scale; }
        core::Vector3      getColliderSize()   const { return m_colliderSize; }
        core::Vector3      getColliderOffset() const { return m_colliderOffset; }

    private:
        // デフォルト値なし - JSON読み込み失敗時はFail Fast
        std::string   m_modelPath;
        core::Vector3 m_position;
        core::Vector3 m_rotation;
        core::Vector3 m_scale;
        core::Vector3 m_colliderSize;
        core::Vector3 m_colliderOffset;
    };
}
