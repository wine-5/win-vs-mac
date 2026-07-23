#pragma once
#include <string>
#include "core/utility/Vector3.h"
#include "core/data/ModelMetadata.h"
#include "game/constant/MetadataKeys.h"
#include "game/data/FileExtensionBonus.h"

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
			data.m_scale = metadata.scale;
			data.m_colliderSize = metadata.colliderSize;
			data.m_colliderOffset = metadata.colliderOffset;

			// moveSpeed（floatProperties から取得）
			auto moveSpeedIt{ metadata.floatProperties.find(
				std::string(constant::metadata_keys::MOVE_SPEED)) };
			if (moveSpeedIt != metadata.floatProperties.end())
				data.m_moveSpeed = moveSpeedIt->second;

			auto dashMultiplierIt{ metadata.floatProperties.find(
				std::string(constant::metadata_keys::DASH_MULTIPLIER)) };
			if (dashMultiplierIt != metadata.floatProperties.end())
				data.m_dashMultiplier = dashMultiplierIt->second;

			auto maxHpIt{ metadata.floatProperties.find(
				std::string(constant::metadata_keys::MAX_HP)) };
			if (maxHpIt != metadata.floatProperties.end())
				data.m_maxHp = maxHpIt->second;

			auto defenceIt{ metadata.floatProperties.find(
				std::string(constant::metadata_keys::DEFENCE)) };
			if (defenceIt != metadata.floatProperties.end())
				data.m_defence = defenceIt->second;

			auto attackPowerIt{ metadata.floatProperties.find(
				std::string(constant::metadata_keys::ATTACK_POWER)) };
			if (attackPowerIt != metadata.floatProperties.end())
				data.m_attackPower = attackPowerIt->second;

			auto attackRangeIt{ metadata.floatProperties.find(
				std::string(constant::metadata_keys::ATTACK_RANGE)) };
			if (attackRangeIt != metadata.floatProperties.end())
				data.m_attackRange = attackRangeIt->second;

			auto attackCooldownIt{ metadata.floatProperties.find(
				std::string(constant::metadata_keys::ATTACK_COOLDOWN)) };
			if (attackCooldownIt != metadata.floatProperties.end())
				data.m_attackCooldown = attackCooldownIt->second;

			// 基本値を保存（ボーナス計算用）
			data.m_baseHp = data.m_maxHp;
			data.m_baseAtk = data.m_attackPower;
			data.m_baseDef = data.m_defence;
			data.m_baseSpd = data.m_moveSpeed;

			return data;
		}

		/** @brief 移動速度を取得 */
		[[nodiscard]] float              getMoveSpeed()      const noexcept { return m_moveSpeed; }
		/** @brief ダッシュ速度倍率を取得 */
		[[nodiscard]] float getDashMultiplier() const noexcept
		{
			return m_dashMultiplier;
		}
		/** @brief 最大HPを取得 */
		[[nodiscard]] float              getMaxHp()          const noexcept { return m_maxHp; }
		/** @brief 防御力を取得 */
		[[nodiscard]] float              getDefence()        const noexcept { return m_defence; }
		/** @brief 攻撃力を取得 */
		[[nodiscard]] float              getAttackPower()    const noexcept { return m_attackPower; }
		/** @brief 攻撃範囲を取得 */
		[[nodiscard]] float              getAttackRange()    const noexcept { return m_attackRange; }
		/** @brief 攻撃クールダウンを取得 */
		[[nodiscard]] float              getAttackCooldown() const noexcept { return m_attackCooldown; }
		/** @brief コライダーサイズを取得 */
		[[nodiscard]] core::Vector3      getColliderSize()   const noexcept { return m_colliderSize; }
		/** @brief コライダーオフセットを取得 */
		[[nodiscard]] core::Vector3      getColliderOffset() const noexcept { return m_colliderOffset; }
		/** @brief モデルスケールを取得 */
		[[nodiscard]] core::Vector3      getScale()          const noexcept { return m_scale; }

		/**
		 * @brief FileExtensionBonus をパラメータに加算する
		 * @param bonus 拡張子ボーナス値
		 * TODO: 今後はプレイヤーのパラメータを直接変更する以外に武器の攻撃力を上げるなどにする
		 */
		void applyExtensionBonus(const data::FileExtensionBonus& bonus)
		{
			m_attackPower += bonus.atk;
			m_moveSpeed += bonus.spd;
			m_defence += bonus.def;
			m_maxHp += bonus.hp;
			m_attackRange += bonus.attackRange;
		}

		/**
		 * @brief 職業パラメータを加算値として適用する
		 * @param hp 職業HP値
		 * @param atk 職業攻撃力
		 * @param def 職業防御力
		 * @param spd 職業移動速度
		 */
		void applyJobParameters(float hp, float atk, float def, float spd) noexcept
		{
			// 職業パラメータを加算値として保存
			m_jobHpAddition = hp - m_baseHp;
			m_jobAtkAddition = atk - m_baseAtk;
			m_jobDefAddition = def - m_baseDef;
			m_jobSpdAddition = spd - m_baseSpd;

			// 実際の値を更新
			m_maxHp = m_baseHp + m_jobHpAddition;
			m_attackPower = m_baseAtk + m_jobAtkAddition;
			m_defence = m_baseDef + m_jobDefAddition;
			m_moveSpeed = m_baseSpd + m_jobSpdAddition;
		}

	private:
		float         m_moveSpeed{ 0.0f };
		float m_dashMultiplier{ 1.0f }; // JSON未設定時はダッシュしても等速
		float         m_maxHp{ 0.0f };
		float         m_defence{ 0.0f };
		float         m_attackPower{ 0.0f };
		float         m_attackRange{ 0.0f };
		float         m_attackCooldown{ 0.0f };
		core::Vector3 m_colliderSize;
		core::Vector3 m_colliderOffset;
		core::Vector3 m_scale{ 1.0f, 1.0f, 1.0f };

		// 基本値（メタデータから取得）
		float m_baseHp{ 0.0f };
		float m_baseAtk{ 0.0f };
		float m_baseDef{ 0.0f };
		float m_baseSpd{ 0.0f };

		// 加算値（職業選択で加算）
		float m_jobHpAddition{ 0.0f };
		float m_jobAtkAddition{ 0.0f };
		float m_jobDefAddition{ 0.0f };
		float m_jobSpdAddition{ 0.0f };
	};
} // namespace game::data