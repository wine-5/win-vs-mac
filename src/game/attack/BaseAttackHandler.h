#pragma once
#include <memory>
#include "IDamageHandler.h"
#include "core/ecs/ComponentManager.h"

namespace game::attack
{
    /**
     * @brief 攻撃者の攻撃力をダメージ初期値としてセットするハンドラ（チェーンの先頭）
     */
    class BaseAttackHandler : public IDamageHandler
    {
    public:
        /**
         * @brief BaseAttackHandlerのコンストラクタ
         * @param componentManager ComponentManagerの参照
         */
        explicit BaseAttackHandler(core::ecs::ComponentManager& componentManager);

        /**
         * @brief 次のハンドラをセットする
         * @param next 次のハンドラ
         */
        void setNext(std::unique_ptr<IDamageHandler> next) override;

        /**
         * @brief 攻撃者の攻撃力をDamageChainにセットし、次のハンドラに渡す
         * @param chain 攻撃計算コンテキスト
         */
        void handle(DamageChain& chain) override;

    private:
        core::ecs::ComponentManager& m_componentManager;
        std::unique_ptr<IDamageHandler> m_next{};
    };
} // namespace game::attack