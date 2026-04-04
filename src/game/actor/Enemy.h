#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/Vector3.h"
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
         * @param modelHandle モデルハンドル
         * @param Enemyのデータ
         */
        Enemy(core::ecs::EntityManager& entityManager,
            core::ecs::ComponentManager& componentManager,
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