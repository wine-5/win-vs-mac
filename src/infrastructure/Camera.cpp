#include "Camera.h"
#include <DxLib.h>

namespace infrastructure
{
    void Camera::update(const core::Vector3& targetPosition, const core::Vector3& offset)
    {
        VECTOR position = { targetPosition.x + offset.x,
                            targetPosition.y + offset.y,
                            targetPosition.z + offset.z };
        VECTOR target = { targetPosition.x, targetPosition.y, targetPosition.z };

        SetCameraPositionAndTarget_UpVecY(position, target);
    }
}