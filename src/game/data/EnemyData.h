#pragma once
#include <string>
#include <vector>
#include <optional>
#include "core/utility/Vector3.h"
#include "core/data/ModelMetadata.h"
#include "core/data/MacMetadata.h"
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
            data.m_scale = metadata.scale;
            data.m_position = metadata.position;
            data.m_colliderSize = metadata.colliderSize;
            data.m_colliderOffset = metadata.colliderOffset;
			data.m_behaviors = metadata.behaviors;   // 積むAI振る舞いのレシピ
			data.m_animations = metadata.animations; // アニメーションクリップ定義
			data.m_mac = metadata.mac;               // ボス挙動定義（あれば）

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

			auto attackWindupIt{ metadata.floatProperties.find(
				std::string(constant::metadata_keys::ATTACK_WINDUP)) };
			if (attackWindupIt != metadata.floatProperties.end())
				data.m_attackWindup = attackWindupIt->second;

			auto hoverHeightIt{ metadata.floatProperties.find(
				std::string(constant::metadata_keys::HOVER_HEIGHT)) };
			if (hoverHeightIt != metadata.floatProperties.end())
				data.m_hoverHeight = hoverHeightIt->second;

			auto preferredDistanceMinIt{ metadata.floatProperties.find(
				std::string(constant::metadata_keys::PREFERRED_DISTANCE_MIN)) };
			if (preferredDistanceMinIt != metadata.floatProperties.end())
				data.m_preferredDistanceMin = preferredDistanceMinIt->second;

			auto preferredDistanceMaxIt{ metadata.floatProperties.find(
				std::string(constant::metadata_keys::PREFERRED_DISTANCE_MAX)) };
			if (preferredDistanceMaxIt != metadata.floatProperties.end())
				data.m_preferredDistanceMax = preferredDistanceMaxIt->second;

			auto fireCooldownIt{ metadata.floatProperties.find(
				std::string(constant::metadata_keys::FIRE_COOLDOWN)) };
			if (fireCooldownIt != metadata.floatProperties.end())
				data.m_fireCooldown = fireCooldownIt->second;

			auto facingYawOffsetIt{ metadata.floatProperties.find(
				std::string(constant::metadata_keys::FACING_YAW_OFFSET)) };
			if (facingYawOffsetIt != metadata.floatProperties.end())
				data.m_facingYawOffset = facingYawOffsetIt->second;

			return data;
        }

		/** @brief 移動速度を取得 */
		[[nodiscard]] float getMoveSpeed() const noexcept
		{
			return m_moveSpeed;
		}

		/** @brief 索敵範囲を取得 */
		[[nodiscard]] float getDetectionRange() const noexcept
		{
			return m_detectionRange;
		}

		/** @brief 攻撃範囲を取得 */
		[[nodiscard]] float getAttackRange() const noexcept
		{
			return m_attackRange;
		}

		/** @brief 最大HPを取得 */
		[[nodiscard]] float getMaxHp() const noexcept
		{
			return m_maxHp;
		}

		/** @brief 防御力を取得 */
		[[nodiscard]] float getDefence() const noexcept
		{
			return m_defence;
		}

		/** @brief 攻撃力を取得 */
		[[nodiscard]] float getAttackPower() const noexcept
		{
			return m_attackPower;
		}

		/** @brief 攻撃クールダウンを取得 */
		[[nodiscard]] float getAttackCooldown() const noexcept
		{
			return m_attackCooldown;
		}

		/** @brief 攻撃のワインドアップ（要求からダメージ判定までの遅延・秒）を取得。0なら即時 */
		[[nodiscard]] float getAttackWindup() const noexcept
		{
			return m_attackWindup;
		}

		/** @brief 浮遊高度を取得（0なら地上型） */
		[[nodiscard]] float getHoverHeight() const noexcept
		{
			return m_hoverHeight;
		}

		/** @brief 維持したい最小距離を取得 */
		[[nodiscard]] float getPreferredDistanceMin() const noexcept
		{
			return m_preferredDistanceMin;
		}

		/** @brief 維持したい最大距離を取得 */
		[[nodiscard]] float getPreferredDistanceMax() const noexcept
		{
			return m_preferredDistanceMax;
		}

		/** @brief 遠距離攻撃（弾発射）の間隔を取得 */
		[[nodiscard]] float getFireCooldown() const noexcept
		{
			return m_fireCooldown;
		}

		/** @brief 正面向きのyawオフセット（度）を取得 */
		[[nodiscard]] float getFacingYawOffset() const noexcept
		{
			return m_facingYawOffset;
		}

		/** @brief コライダーサイズを取得 */
		[[nodiscard]] core::Vector3 getColliderSize() const noexcept
		{
			return m_colliderSize;
		}

		/** @brief コライダーオフセットを取得 */
		[[nodiscard]] core::Vector3 getColliderOffset() const noexcept
		{
			return m_colliderOffset;
		}

		/** @brief モデルスケールを取得 */
		[[nodiscard]] core::Vector3 getScale() const noexcept
		{
			return m_scale;
		}

		/** @brief 初期位置を取得 */
		[[nodiscard]] core::Vector3 getPosition() const noexcept
		{
			return m_position;
		}

		/** @brief 初期位置を設定する（ステージ配置定義から上書きする用） */
		void setPosition(const core::Vector3& position) noexcept
		{
			m_position = position;
		}

		/** @brief 積むAI振る舞いのレシピ（名前リスト）を取得 */
		[[nodiscard]] const std::vector<std::string>& getBehaviors() const noexcept
		{
			return m_behaviors;
		}

		/** @brief ボス挙動定義を取得（bossを持たない敵ではnullopt） */
		[[nodiscard]] const std::optional<core::data::MacMetadata>& getMac() const noexcept
		{
			return m_mac;
		}

		/** @brief アニメーションクリップ定義の一覧を取得（アニメ無しの敵では空） */
		[[nodiscard]] const std::vector<core::data::AnimationClipDef>& getAnimations() const noexcept
		{
			return m_animations;
		}

	private:
	  float m_moveSpeed{ 0.0f };
	  float m_detectionRange{ 0.0f };
	  float m_attackRange{ 0.0f };
	  float m_maxHp{ 0.0f };
	  float m_defence{ 0.0f };
	  float m_attackPower{ 0.0f };
	  float m_attackCooldown{ 0.0f };
	  float m_attackWindup{ 0.0f };
	  float m_hoverHeight{ 0.0f };
	  float m_preferredDistanceMin{ 0.0f };
	  float m_preferredDistanceMax{ 0.0f };
	  float m_fireCooldown{ 0.0f };
	  float m_facingYawOffset{ 0.0f };
	  core::Vector3 m_colliderSize;
	  core::Vector3 m_colliderOffset;
	  core::Vector3 m_scale{ 1.0f, 1.0f, 1.0f };
	  core::Vector3 m_position;
	  std::vector<std::string> m_behaviors{};                   // 積むAI振る舞いのレシピ
	  std::optional<core::data::MacMetadata> m_mac{};           // ボス挙動定義（あれば）
	  std::vector<core::data::AnimationClipDef> m_animations{}; // アニメーションクリップ定義
	};
} // namespace game::data