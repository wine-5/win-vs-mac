#pragma once
#include <string>
#include "core/Vector3.h"
#include "core/data/ModelMetadata.h"
#include "game/constant/MetadataKeys.h"

namespace game::data
{
    /**
     * @brief Enemyのデータを保持するクラス
     */
    class EnemyData
    {
    public:
        /**
        * @brief ModelMetadataからEnemyDataを生成
        * @param metadata ResourceManagerから取得したメタデータ
        * @return EnemyDataインスタンス
        */
        static EnemyData fromMetadata(const core::data::ModelMetadata& metadata)
        {
            EnemyData data;
            data.m_modelPath = metadata.modelPath;
            data.m_colliderSize = metadata.colliderSize;
            data.m_colliderOffset = metadata.colliderOffset;

            auto idleIt{metadata.stringProperties.find(
                std::string(constant::metadata_keys::IDLE_ANIM))};
            if (idleIt != metadata.stringProperties.end())
                data.m_idleAnimPath = idleIt->second;

            auto walkIt{metadata.stringProperties.find(
                std::string(constant::metadata_keys::WALK_ANIM))};
            if (walkIt != metadata.stringProperties.end())
                data.m_walkAnimPath = walkIt->second;

            auto moveSpeedIt{metadata.floatProperties.find(
                std::string(constant::metadata_keys::MOVE_SPEED))};
            if (moveSpeedIt != metadata.floatProperties.end())
                data.m_moveSpeed = moveSpeedIt->second;

            auto detectionRangeIt{metadata.floatProperties.find(
                std::string(constant::metadata_keys::DETECTION_RANGE))};
            if (detectionRangeIt != metadata.floatProperties.end())
                data.m_detectionRange = detectionRangeIt->second;

            auto attackRangeIt{ metadata.floatProperties.find(
                std::string(constant::metadata_keys::ATTACK_RANGE)) };
            if (attackRangeIt != metadata.floatProperties.end())
                data.m_attackRange = attackRangeIt->second;

            return data;
        }

        /** @brief モデルパスを取得 */
        [[nodiscard]] const std::string& getModelPath()      const noexcept { return m_modelPath; }
        /** @brief Idleアニメーションパスを取得 */
        [[nodiscard]] const std::string& getIdleAnimPath()   const noexcept { return m_idleAnimPath; }
        /** @brief Walkアニメーションパスを取得 */
        [[nodiscard]] const std::string& getWalkAnimPath()   const noexcept { return m_walkAnimPath; }
        /** @brief 移動速度を取得 */
        [[nodiscard]] float              getMoveSpeed()      const noexcept { return m_moveSpeed; }
        /** @brief 索敵範囲を取得 */
        [[nodiscard]] float              getDetectionRange() const noexcept { return m_detectionRange; }
        /** @brief 攻撃範囲を取得 */
        [[nodiscard]] float getAttackRange() const noexcept { return m_attackRange; }
        /** @brief コライダーサイズを取得 */
        [[nodiscard]] core::Vector3      getColliderSize()   const noexcept { return m_colliderSize; }
        /** @brief コライダーオフセットを取得 */
        [[nodiscard]] core::Vector3      getColliderOffset() const noexcept { return m_colliderOffset; }

    private:
        std::string m_modelPath;
        std::string m_idleAnimPath;
        std::string m_walkAnimPath;
       float m_attackRange{ 100.0f };
        float m_moveSpeed{ 2.0f };
        float m_detectionRange{ 10.0f };
        core::Vector3 m_colliderSize;
        core::Vector3 m_colliderOffset;
    };
}