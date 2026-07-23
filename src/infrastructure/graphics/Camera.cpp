#include "Camera.h"
#include <DxLib.h>

namespace infrastructure::graphics
{
	void Camera::setLookAt(const core::Vector3& position, const core::Vector3& target)
	{
		VECTOR pos = { position.x, position.y, position.z };
		VECTOR tgt = { target.x, target.y, target.z };
		SetCameraPositionAndTarget_UpVecY(pos, tgt);
	}

	void Camera::setFieldOfView(float fovRad)
	{
		SetupCamera_Perspective(fovRad);
	}
} // namespace infrastructure::graphics
