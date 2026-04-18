#pragma once
#include <string>
#include "core/Vector3.h"
#include "core/data/ModelMetadata.h"
#include "game/constant/MetadataKeys.h"

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
            auto idleIt{metadata.stringProperties.find(
                std::string(constant::metadata_keys::IDLE_ANIM))};
            if (idleIt != metadata.stringProperties.end())
                data.m_idleAnimPath = idleIt->second;

            auto walkIt{metadata.stringProperties.find(
                std::string(constant::metadata_keys::WALK_ANIM))};
            if (walkIt != metadata.stringProperties.end())
                data.m_walkAnimPath = walkIt->second;

            // moveSpeed（floatProperties から取得）
            auto moveSpeedIt{metadata.floatProperties.find(
                std::string(constant::metadata_keys::MOVE_SPEED))};
            if (moveSpeedIt != metadata.floatProperties.end())
                data.m_moveSpeed = moveSpeedIt->second;

            return data;
        }

        /** @brief モデルパスを取得 */
        [[nodiscard]] const std::string& getModelPath()    const noexcept { return m_modelPath; }
        /** @brief Idleアニメーションパスを取得 */
        [[nodiscard]] const std::string& getIdleAnimPath() const noexcept { return m_idleAnimPath; }
        /** @brief Walkアニメーションパスを取得 */
        [[nodiscard]] const std::string& getWalkAnimPath() const noexcept { return m_walkAnimPath; }
        /** @brief 移動速度を取得 */
        [[nodiscard]] float              getMoveSpeed()    const noexcept { return m_moveSpeed; }
        /** @brief コライダーサイズを取得 */
        [[nodiscard]] core::Vector3      getColliderSize() const noexcept { return m_colliderSize; }
        /** @brief コライダーオフセットを取得 */
        [[nodiscard]] core::Vector3      getColliderOffset() const noexcept { return m_colliderOffset; }

    private:
        // JSON読み込み失敗時はFail Fast
        std::string   m_modelPath;
        std::string   m_idleAnimPath;
        std::string   m_walkAnimPath;
        float         m_moveSpeed{0.0f};
        core::Vector3 m_colliderSize;
        core::Vector3 m_colliderOffset;
    };
}