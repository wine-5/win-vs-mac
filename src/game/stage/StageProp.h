#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"
#include "core/data/FileExtensionType.h"
#include "game/constant/EnemyType.h"
#include "game/constant/PropCollision.h"
#include <vector>

namespace game::stage
{
	/**
	 * @brief StagePropの生成に必要な値一式
	 *
	 * 位置・回転・スケール・当たり判定・テクスチャ繰り返しと項目が多いため、
	 * 引数の並び間違いを防ぐ目的でまとめている。
	 */
	struct StagePropParams
	{
		int m_modelHandle{ -1 };
		core::Vector3 m_position{};
		core::Vector3 m_rotation{}; // ラジアン
		core::Vector3 m_scale{ 1.0f, 1.0f, 1.0f };

		constant::PropCollision m_collision{ constant::PropCollision::None };
		core::Vector3 m_collisionSize{}; // Boxならコライダー、Groundなら歩ける面の実寸

		float m_uvScaleU{ 1.0f }; // テクスチャの繰り返し回数（1.0で引き伸ばし）
		float m_uvScaleV{ 1.0f };

		float m_slideAccel{ 0.0f };    // 坂を滑り落ちる加速度（Groundのみ有効。0で滑らない）
		float m_conveyorSpeed{ 0.0f }; // 動く歩道の速さ（Groundのみ有効。0で運ばない）

		float m_scrollSpeedU{ 0.0f }; // テクスチャを流す速さ（1.0でテクスチャ1枚ぶん/秒）
		float m_scrollSpeedV{ 0.0f };

		// 破壊に必要な打撃回数。0なら壊せない普通の配置物として作る
		int m_hitsToBreak{ 0 };

		// 破壊した瞬間へ差し替える「割ってあるモデル」のハンドル
		int m_fracturedHandle{ -1 };

		// ひび段階のテクスチャ。[0]が無傷で、以降が段階1〜のひび
		std::vector<int> m_crackTextures{};

		// 壊したときに落とす拡張子の種別と個数。種別を抽選する場合は m_isDropRandom
		core::data::FileExtensionType m_dropType{ core::data::FileExtensionType::Unknown };
		int m_dropCount{ 0 };
		bool m_isDropRandom{ false };

		// 壊すと拡張子を挿せる枠が1つ増えるか（RAMブロック）
		bool m_grantsEquipSlot{ false };

		// 壊したときに湧く敵の種類と数（ギャンブルボックス）。0体なら敵は出ない
		constant::EnemyType m_spawnEnemyType{ constant::EnemyType::Xcode };
		int m_spawnEnemyCount{ 0 };

		// 敵の代わりに当たりを引く確率と、当たったときの能力倍率
		float m_extensionBoostChance{ 0.0f };
		float m_extensionBoostMultiplier{ 1.0f };
	};

	/**
	 * @brief ステージの配置物（床・壁・柱など）1つ分のオブジェクト
	 *
	 * 立方体モデルをXYZに引き伸ばして床・通路・壁・柱を賄うグレーボックス方式。
	 * 解決済みの値を受け取り、ここではEntityへのComponent組み立てのみを行う。
	 */
	class StageProp
	{
	  public:
		/**
		 * @brief StagePropのコンストラクタ
		 * @param entityManager EntityManagerの参照
		 * @param componentManager ComponentManagerの参照
		 * @param params 生成に必要な値一式
		 */
		StageProp(core::ecs::EntityManager& entityManager,
		    core::ecs::ComponentManager& componentManager,
		    const StagePropParams& params);

		/**
		 * @brief StagePropのEntityIDを取得する
		 * @return EntityID
		 */
		core::ecs::EntityId getId() const noexcept;

	  private:
		core::ecs::Entity m_entity;
	};
} // namespace game::stage
