#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/utility/Vector3.h"
#include "game/constant/Tag.h"

namespace game::factory
{
	/**
	 * @brief 弾の生成パラメータ
	 */
	struct ProjectileConfig
	{
		float m_speed{ 0.0f };    // 進行速度
		float m_damage{ 0.0f };   // 与えるダメージ（AttackComponent.m_attackPower に入る）
		float m_lifetime{ 0.0f }; // 寿命
		float m_radius{ 0.0f };   // 大きさの半径（AttackComponent.m_attackRange に入る＝当たり判定）
		float m_scale{ 1.0f };    // 見た目スケール（0だと描画されないため1.0を既定にする）
		int m_modelHandle{ -1 };  // 3Dモデル描画用のハンドル（-1ならモデル描画なし）

		// 0より大きければルーレット回転（画面正対のZ軸スピン）で描画する。
		// 値は1ワールド単位進むごとの回転量[rad]（レインボーの演出用）
		float m_spinRollSpeed{ 0.0f };
	};

	/**
	 * @brief 弾エンティティを生成するファクトリ
	 *
	 * 既存Componentの組み合わせで弾を構築する。プレイヤー・敵の遠距離攻撃で共通利用する。
	 */
	class ProjectileFactory
	{
	  public:
		/**
		 * @brief ProjectileFactoryのコンストラクタ
		 * @param entityManager EntityManagerの参照
		 * @param componentManager ComponentManagerの参照
		 */
		ProjectileFactory(core::ecs::EntityManager& entityManager,
		    core::ecs::ComponentManager& componentManager);

		/**
		 * @brief 弾を生成する
		 * @param origin 発射位置
		 * @param direction 進行方向（単位ベクトル）
		 * @param config 弾のパラメータ
		 * @param ownerTag 発射者の陣営（誤爆防止用）
		 * @return 生成した弾のEntityID
		 */
		core::ecs::EntityId spawn(const core::Vector3& origin,
		    const core::Vector3& direction,
		    const ProjectileConfig& config,
		    constant::Tag ownerTag);

	  private:
		core::ecs::EntityManager& m_entityManager;
		core::ecs::ComponentManager& m_componentManager;
	};
} // namespace game::factory
