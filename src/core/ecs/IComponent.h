#pragma once

namespace core::ecs
{
    /**
     * @brief 型消去用の基底インターフェース
     */
    class IComponent
    {
    public:
        virtual ~IComponent() = default;
        
        /**
         * @brief EntityのComponentを削除する
         * @param id EntityID
         */
        virtual void remove(EntityId id) = 0;
    };
}