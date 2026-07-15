#pragma once
#include <memory>
#include "IDamageHandler.h"
#include "core/ecs/ComponentManager.h"

namespace game::attack
{
    /**
     * @brief 被攻撃者の防御力をダメージから減算するハンドラ
     */
    class DefenseHandler : public IDamageHandler
    {
    public:
        /**
         * @brief DefenseHandlerのコンストラクタ
         * @param componentManager ComponentManagerの参照
         */
        explicit DefenseHandler(core::ecs::ComponentManager& componentManager);

        /**
         * @brief 次のハンドラをセットする
         * @param next 次のハンドラ
         */
        void setNext(std::unique_ptr<IDamageHandler> next) override;

        /**
         * @brief 被攻撃者の防御力をダメージから減算し、次のハンドラに渡す
         * @param chain 攻撃計算コンテキスト
         */
        void handle(DamageChain& chain) override;

    private:
        core::ecs::ComponentManager& m_componentManager;
        std::unique_ptr<IDamageHandler> m_next{};
    };
} // namespace game::attack