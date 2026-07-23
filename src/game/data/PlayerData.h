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
		 *
		 * 方針: 装備スロットごとにこの関数を呼んで実値へ加算していく形を維持する。
		 * ベース値と修正値を分けて保持する仕組み（StatModifier等）は、実行時に
		 * 「基礎値＋ボーナス」を分けて見せる必要が出るまで導入しない。
		 * セレクト画面の内訳表示は Win32SelectWindowManager 側が別途計算しており、
		 * ここでは最終値だけを持てば足りる。
		 * @param bonus 拡張子ボーナス値
		 */
		void applyExtensionBonus(const data::FileExtensionBonus& bonus)
		{
			m_attackPower += bonus.atk;
			m_moveSpeed += bonus.spd;
			m_defence += bonus.def;
			m_maxHp += bonus.hp;
			m_attackRange += bonus.attackRange;
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
	};
} // namespace game::data