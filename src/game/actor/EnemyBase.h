#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IResourceManager.h"
#include "game/data/EnemyData.h"

namespace game::actor
{
	/**
	 * @brief 敵の共通セットアップを担う基底クラス
	 *
	 * 責務はスポーン時のコンポーネント構築までに限定する。
	 * 毎フレームの行動ロジックはSystem側（AISystem等）が担当する。
	 * コンストラクタ内から仮想関数を呼んでも派生クラスに届かないため、
	 * 生成後に initialize() を呼ぶ2段階初期化とする
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

	protected:
		/**
		 * @brief アニメーションクリップを登録する（派生クラスが実装）
		 *
		 * アニメーションを持たない敵（Safari等）は空実装でよい
		 */
		virtual void setupAnimation() = 0;

		/**
		 * @brief AIの行動タイプ・パラメータを設定する（派生クラスが実装）
		 */
		virtual void setupAI() = 0;

		core::ecs::Entity m_entity;
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IResourceManager& m_resourceManager;
		int m_modelHandle{ -1 };
		data::EnemyData m_enemyData;

	private:
		/**
		 * @brief 全敵種共通のコンポーネントを構築する
		 */
		void buildCommonComponents();
	};
} // namespace game::actor