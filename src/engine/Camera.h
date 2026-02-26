#pragma once
#include "core/Vector3.h"

namespace engine
{
    /**
     * @brief アイソメトリックカメラの制御クラス
     */
    class Camera
    {
    public:
        void update(const core::Vector3& targetPosition, const core::Vector3& offset);
    };
}