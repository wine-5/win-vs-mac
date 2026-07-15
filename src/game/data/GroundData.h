#pragma once
#include <string>
#include "core/utility/Vector3.h"
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

        /** @brief モデルパスを取得 */
        const std::string& getModelPath()      const noexcept { return m_modelPath; }
        /** @brief 位置を取得 */
        core::Vector3      getPosition()       const noexcept { return m_position; }
        /** @brief 回転を取得 */
        core::Vector3      getRotation()       const noexcept { return m_rotation; }
        /** @brief スケールを取得 */
        core::Vector3      getScale()          const noexcept { return m_scale; }
        /** @brief コライダーサイズを取得 */
        core::Vector3      getColliderSize()   const noexcept { return m_colliderSize; }
        /** @brief コライダーオフセットを取得 */
        core::Vector3      getColliderOffset() const noexcept { return m_colliderOffset; }

    private:
        // デフォルト値なし - JSON読み込み失敗時はFail Fast
        std::string   m_modelPath;
        core::Vector3 m_position;
        core::Vector3 m_rotation;
        core::Vector3 m_scale;
        core::Vector3 m_colliderSize;
        core::Vector3 m_colliderOffset;
    };
} // namespace game::data
