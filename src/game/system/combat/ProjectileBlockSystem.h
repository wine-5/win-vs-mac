#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/EntityManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"
#include <vector>

namespace game::system::combat
{
	/**
	 * @brief 壁・ブロックにぶつかった弾を消すSystem
	 *
	 * 弾は押し返しの対象（CollisionSystemのrider）ではないため、何もしなければ
	 * 遮蔽物をすり抜けて当たってしまう。ここで遮蔽物との接触を見て弾を消し、
	 * 「壁に隠れれば撃たれない」を成立させる。
	 *
	 * 貫通する弾（溜め切ったWindow弾）だけは例外として通す。
	 */
	class ProjectileBlockSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief コンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param entityManager 弾を破棄するためのEntityManagerの参照
		 */
		ProjectileBlockSystem(core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityManager& entityManager);

		/**
		 * @brief 遮蔽物にぶつかった弾を破棄する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		/** @brief 判定に使う遮蔽物の箱（Y軸まわりの傾きを持つ） */
		struct Box
		{
			core::Vector3 m_center{};
			core::Vector3 m_halfSize{};
			float m_yaw{ 0.0f };
		};

		/**
		 * @brief 弾を遮る配置物（ColliderComponentを持つGround）を集める
		 */
		void collectBlockers();

		/**
		 * @brief 線分が箱と交差するか
		 * @param box 判定する箱
		 * @param from 線分の始点（前フレームの弾の位置）
		 * @param to 線分の終点（現在の弾の位置）
		 * @param radius 弾の半径（箱を膨らませて球として扱う）
		 * @return 交差する場合true
		 */
		static bool intersects(const Box& box, const core::Vector3& from, const core::Vector3& to,
		    float radius) noexcept;

		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityManager& m_entityManager;

		std::vector<Box> m_blockers; // 毎フレーム作り直す遮蔽物の一覧（確保済み容量を使い回す）
	};
} // namespace game::system::combat
