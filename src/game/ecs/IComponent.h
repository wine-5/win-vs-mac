#pragma once

namespace game::ecs
{
    /**
     * @brief 型消去用の基底インターフェース
     */
    class IComponent
    {
    public:
        virtual ~IComponent() = default;
    };
}