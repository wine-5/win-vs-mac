#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/interface/IResourceManager.h"
#include "game/actor/EnemyBase.h"
#include "game/data/EnemyData.h"
#include <memory>
#include <vector>

namespace game::factory
{
	/**
	 * @brief EnemyType→具体クラスのインスタンス化と、生成後の寿命管理を担当する低レベルなFactory
	 *
	 * 「型を渡されたら対応するEnemyを作って所有する」ことだけに責務を絞る。
	 * 生成に必要な材料（modelHandle・EnemyData）は呼び出し側で用意済みの前提とし、
	 * リソースのロードやステージ配置、AI追跡対象の設定などゲーム文脈は一切知らない。
	 * これによりResourceManager等への依存が広がらず、生成の抽象として単体テストしやすく保つ。
	 *
	 * ゲーム文脈を組み立てて本クラスを呼び出す高レベルな役割は EnemySpawner が担う。
	 * （生成の「オーケストレーション」と「インスタンス化」を意図的に分離している）
	 */
	class EnemyFactory
	{
    public:
        /**
         * @brief EnemyFactoryのコンストラクタ
         * @param entityManager EntityManagerの参照
         * @param componentManager ComponentManagerの参照
         * @param resourceManager IResourceManagerの参照
         */
        EnemyFactory(
            core::ecs::EntityManager& entityManager,
            core::ecs::ComponentManager& componentManager,
            core::iface::IResourceManager& resourceManager);

		/**
		 * @brief Enemyオブジェクトを生成する（中身はenemyDataのレシピが決める）
		 * @param modelHandle モデルハンドル
		 * @param enemyData Enemyのデータ（behaviors/animations等のレシピを含む）
		 * @return 生成したEnemyのEntityId
		 */
		core::ecs::EntityId create(int modelHandle, const data::EnemyData& enemyData);

		/**
		 * @brief 内部で保持しているEnemyを破棄する（死亡後の後始末で呼ぶ）
		 *
		 * Entity/Componentの破棄はこのクラスの責務外。あくまで内部リスト
		 * （m_enemies）からの除去のみを行う
		 * @param id 除去するEnemyのEntityId
		 */
		void remove(core::ecs::EntityId id);

	  private:
        core::ecs::EntityManager& m_entityManager;
        core::ecs::ComponentManager& m_componentManager;
        core::iface::IResourceManager& m_resourceManager;

        std::vector<std::unique_ptr<actor::EnemyBase>> m_enemies;
    };
} // namespace game::factory