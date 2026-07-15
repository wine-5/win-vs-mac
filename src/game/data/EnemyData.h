#pragma once
#include <string>
#include "core/utility/Vector3.h"
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
            data.m_scale = metadata.scale;
            data.m_position = metadata.position;
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

            auto attackRangeIt{metadata.floatProperties.find(
                std::string(constant::metadata_keys::ATTACK_RANGE))};
            if (attackRangeIt != metadata.floatProperties.end())
                data.m_attackRange = attackRangeIt->second;

            auto maxHpIt{metadata.floatProperties.find(
                std::string(constant::metadata_keys::MAX_HP))};
            if (maxHpIt != metadata.floatProperties.end())
                data.m_maxHp = maxHpIt->second;

            auto defenceIt{metadata.floatProperties.find(
                std::string(constant::metadata_keys::DEFENCE))};
            if (defenceIt != metadata.floatProperties.end())
                data.m_defence = defenceIt->second;

            auto attackPowerIt{metadata.floatProperties.find(
                std::string(constant::metadata_keys::ATTACK_POWER))};
            if (attackPowerIt != metadata.floatProperties.end())
                data.m_attackPower = attackPowerIt->second;

            auto attackCooldownIt{metadata.floatProperties.find(
                std::string(constant::metadata_keys::ATTACK_COOLDOWN))};
            if (attackCooldownIt != metadata.floatProperties.end())
                data.m_attackCooldown = attackCooldownIt->second;

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
        [[nodiscard]] float              getAttackRange()    const noexcept { return m_attackRange; }
        /** @brief 最大HPを取得 */
        [[nodiscard]] float              getMaxHp()          const noexcept { return m_maxHp; }
        /** @brief 防御力を取得 */
        [[nodiscard]] float              getDefence()        const noexcept { return m_defence; }
        /** @brief 攻撃力を取得 */
        [[nodiscard]] float              getAttackPower()    const noexcept { return m_attackPower; }
        /** @brief 攻撃クールダウンを取得 */
        [[nodiscard]] float              getAttackCooldown() const noexcept { return m_attackCooldown; }
        /** @brief コライダーサイズを取得 */
        [[nodiscard]] core::Vector3      getColliderSize()   const noexcept { return m_colliderSize; }
        /** @brief コライダーオフセットを取得 */
        [[nodiscard]] core::Vector3      getColliderOffset() const noexcept { return m_colliderOffset; }
        /** @brief モデルスケールを取得 */
        [[nodiscard]] core::Vector3      getScale()          const noexcept { return m_scale; }
        /** @brief 初期位置を取得 */
        [[nodiscard]] core::Vector3      getPosition()       const noexcept { return m_position; }
        /** @brief 初期位置を設定する（ステージ配置定義から上書きする用） */
        void setPosition(const core::Vector3& position) noexcept { m_position = position; }

    private:
        std::string   m_modelPath;
        std::string   m_idleAnimPath;
        std::string   m_walkAnimPath;
        float         m_moveSpeed{ 0.0f };
        float         m_detectionRange{ 0.0f };
        float         m_attackRange{ 0.0f };
        float         m_maxHp{ 0.0f };
        float         m_defence{ 0.0f };
        float         m_attackPower{ 0.0f };
        float         m_attackCooldown{ 0.0f };
        core::Vector3 m_colliderSize;
        core::Vector3 m_colliderOffset;
        core::Vector3 m_scale{ 1.0f, 1.0f, 1.0f };
        core::Vector3 m_position;
    };
} // namespace game::data