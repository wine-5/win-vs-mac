#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IResourceManager.h"
#include "game/data/EnemyData.h"

namespace game::actor
{
	/**
	 * @brief データ駆動の汎用Enemy。敵は「モデル＋behaviors＋animationsの組み合わせ」で定義する
	 *
	 * 敵種ごとの派生クラスは廃止し、生成に必要な差分はすべてEnemyData（JSON由来）が持つ。
	 * initialize()で共通コンポーネントを構築し、アニメ・AI振る舞いをレシピから積む。
	 * 責務はスポーン時のコンポーネント構築までに限定し、毎フレームの行動はSystem側が担う。
	 */
	class EnemyBase
	{
	public:
		/**
		 * @brief EnemyBaseのコンストラクタ
		 * @param entityManager EntityManagerの参照
		 * @param componentManager ComponentManagerの参照
		 * @param resourceManager アニメーションハンドル取得用のIResourceManager
		 * @param modelHandle モデルハンドル
		 * @param enemyData 敵のデータ
		 */
	  EnemyBase(core::ecs::EntityManager& entityManager,
		        core::ecs::ComponentManager& componentManager,
		        core::iface::IResourceManager& resourceManager,
		        int modelHandle,
		        data::EnemyData enemyData);

	  virtual ~EnemyBase() = default;

	  /**
	   * @brief コンポーネントを構築する（生成直後に必ず呼ぶ）
	   *
	   * 共通コンポーネントの構築後、派生クラスのフック
	   * （setupAnimation / setupAI）を呼び出す
	   */
	  void initialize();

	  /**
	   * @brief 敵のEntityIDを取得する
	   * @return EntityID
	   */
	  [[nodiscard]] core::ecs::EntityId getId() const noexcept;

	private:
	  /**
	   * @brief 全敵種共通のコンポーネントを構築する
	   */
	  void buildCommonComponents();

	  core::ecs::Entity m_entity;
	  core::ecs::ComponentManager& m_componentManager;
	  core::iface::IResourceManager& m_resourceManager;
	  int m_modelHandle{ -1 };
	  data::EnemyData m_enemyData;
	};
} // namespace game::actor