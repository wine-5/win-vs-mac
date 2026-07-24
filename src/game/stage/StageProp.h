#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"
#include "game/constant/PropCollision.h"

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

		float m_slideAccel{ 0.0f }; // 坂を滑り落ちる加速度（Groundのみ有効。0で滑らない）
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
