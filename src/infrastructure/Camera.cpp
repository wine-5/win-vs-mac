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

		// デバック：動いているのが分かるように一時的にカメラを固定するテストコード
		/*VECTOR position = { offset.x, offset.y, offset.z };
		VECTOR target = { 0.0f, 0.0f, 0.0f };*/

		SetCameraPositionAndTarget_UpVecY(position, target);
	}
} // namespace infrastructure