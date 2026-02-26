#pragma once
#include "core/Vector3.h"

namespace game::ecs::component
{
    /**
     * @brief 速度を持つコンポーネント
     */
    struct VelocityComponent
    {
        core::Vector3 m_velocity;
    };
}