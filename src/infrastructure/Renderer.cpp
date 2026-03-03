#include "Renderer.h"
#include "DxLib.h"
#include "core/interface/ILogger.h"

namespace infrastructure
{
	void Renderer::drawModel(int modelHandle, const core::Vector3& position, const core::Vector3& rotation)
	{
		if (modelHandle == -1)
		{
			LOG_E("モデルが読み込まれていません");
			return;
		}

		VECTOR pos = { position.x, position.y, position.z };
		VECTOR rot = { rotation.x, rotation.y, rotation.z };

		MV1SetPosition(modelHandle, pos);
		MV1SetRotationXYZ(modelHandle, rot);
		MV1DrawModel(modelHandle);
	}
}