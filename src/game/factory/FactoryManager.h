#pragma once
#include "PlayerFactory.h"
#include "GroundFactory.h"
#include "EnemyFactory.h"

namespace game::factory
{
    /**
     * @brief 全てのFactoryを統括管理するFacadeクラス
     */
    class FactoryManager
    {
    public:
        /**
         * @brief FactoryManagerのコンストラクタ
         * @param entityManager EntityManagerの参照
         * @param componentManager ComponentManagerの参照
         * @param resourceManager IResourceManagerの参照
         */
        FactoryManager(
            core::ecs::EntityManager& entityManager,
            core::ecs::ComponentManager& componentManager,
            core::iface::IResourceManager& resourceManager);

        /**
         * @brief PlayerFactoryを取得する
         * @return PlayerFactoryの参照
         */
        [[nodiscard]] PlayerFactory& getPlayerFactory() const noexcept;

        /**
         * @brief GroundFactoryを取得する
         * @return GroundFactoryの参照
         */
        [[nodiscard]] GroundFactory& getGroundFactory() const noexcept;

        /**
         * @brief EnemyFactoryを取得する
         * @return EnemyFactoryの参照
         */
        [[nodiscard]] EnemyFactory& getEnemyFactory() const noexcept;

    private:
        std::unique_ptr<PlayerFactory> m_playerFactory;
        std::unique_ptr<GroundFactory> m_groundFactory;
        std::unique_ptr<EnemyFactory> m_enemyFactory;
    };
}