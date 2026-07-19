#pragma once
#include "IFactory.h"
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/interface/IResourceManager.h"
#include "game/actor/EnemyBase.h"
#include "game/data/EnemyData.h"
#include "game/constant/EnemyType.h"
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
	class EnemyFactory : public IFactory
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
          * @brief 種類を指定してEnemyオブジェクトを生成する
          * @param type 敵の種類
          * @param modelHandle モデルハンドル
          * @param enemyData Enemyのデータ
          * @return 生成したEnemyのEntityId
          */
        core::ecs::EntityId create(constant::EnemyType type, int modelHandle, const data::EnemyData& enemyData);

        /**
         * @brief 生成した全EnemyのEntityIdを取得する
         * @return EntityIdのvector
         */
        [[nodiscard]] const std::vector<core::ecs::EntityId>& getEnemyIds() const noexcept;

    private:
        core::ecs::EntityManager& m_entityManager;
        core::ecs::ComponentManager& m_componentManager;
        core::iface::IResourceManager& m_resourceManager;

        std::vector<std::unique_ptr<actor::EnemyBase>> m_enemies;
        std::vector<core::ecs::EntityId> m_enemyIds{};
    };
} // namespace game::factory