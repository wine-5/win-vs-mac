#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"
#include <vector>

// 前方宣言（実体は .cpp でインクルード）
namespace game::component::movement
{
	struct TransformComponent;
	struct VelocityComponent;
} // namespace game::component::movement

namespace game::system::combat
{
    /**
     * @brief ColliderComponentを持つ全エンティティ間の衝突検出と押し返しを行うSystem
     */
    class CollisionSystem : public core::ecs::ISystem
    {
    public:
		/**
		 * @brief CollisionSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 */
		CollisionSystem(core::ecs::ComponentManager& componentManager);

		/**
		 * @brief 全Entity間の衝突検出と押し返しを行う
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	private:
	  /**
	   * @brief 判定に必要な値だけを抜き出した、1フレーム限りのAABB情報
	   *
	   * 総当たりの内側でComponentを引き直すとハッシュ検索が要素数の二乗で効いてくるため、
	   * フレーム頭に一度だけ集めてこの形に持つ。
	   */
	  struct Aabb
	  {
		  core::ecs::EntityId m_id{};
		  core::Vector3 m_center{};   // position + offset
		  core::Vector3 m_halfSize{}; // size / 2
	  };

	  /**
	   * @brief 衝突判定の対象を「乗る側」と「地面側」に振り分けて集める
	   *
	   * 押し返しが起きるのは 乗る側×地面側 の組み合わせだけなので、
	   * 事前に分けておき総当たりの回数そのものを減らす。
	   */
	  void collectAabbs();

	  /**
	   * @brief 2つのAABBが重なっているか判定する
	   * @param a AABB A
	   * @param b AABB B
	   * @return 衝突している場合true
	   */
	  static bool isColliding(const Aabb& a, const Aabb& b) noexcept;

	  /**
	   * @brief 衝突を解決し、押し返し処理を行う
	   * @param rider 乗る側のAABB（押し出した分だけ中心が更新される）
	   * @param ground 地面側のAABB
	   */
	  void resolveCollision(Aabb& rider, const Aabb& ground);

	  /**
	   * @brief 縦方向（上下）の押し出しを解決する
	   *
	   * riderが上なら地面に乗せ（着地・死亡バウンド処理を含む）、下なら天井として押し戻す。
	   * @param riderId 乗る側のEntityID
	   * @param riderTransform 乗る側のTransform
	   * @param riderVelocity 乗る側のVelocity
	   * @param overlapY Y軸のめり込み量（正）
	   * @param deltaY 乗る側中心 − 相手中心 のY成分（符号で上下を判定）
	   */
	  void resolveVertical(core::ecs::EntityId riderId,
		  component::movement::TransformComponent& riderTransform,
		  component::movement::VelocityComponent& riderVelocity,
		  float overlapY, float deltaY);

	  core::ecs::ComponentManager& m_componentManager;

	  // 毎フレーム作り直すが、確保済みメモリを使い回すためメンバに持つ
	  std::vector<Aabb> m_riders;  // Player / Enemy
	  std::vector<Aabb> m_grounds; // Ground
	};
} // namespace game::system::combat