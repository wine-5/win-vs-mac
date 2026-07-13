#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"
#include "core/interface/IResourceManager.h"
#include "game/data/EnemyData.h"

namespace game::actor
{
    /**
     * @brief Enemyのセットアップを担当するクラス
     */
    class Enemy
    {
    public:
        /**
         * @brief Enemyのコンストラクタ
         * @param entityManager EntityManagerの参照
         * @param componentManager ComponentManagerの参照
         * @param resourceManager アニメーションハンドル取得用のIResourceManager
         * @param modelHandle モデルハンドル
         * @param enemyData Enemyのデータ
         */
        Enemy(core::ecs::EntityManager& entityManager,
            core::ecs::ComponentManager& componentManager,
            core::iface::IResourceManager& resourceManager,
            int modelHandle,
            const data::EnemyData& enemyData
            );

        /**
         * @brief EnemyのEntityIDを取得する
         * @return EntityID
         */
        [[nodiscard]] core::ecs::EntityId getId() const noexcept;

    private:
        core::ecs::Entity m_entity;
    };
}