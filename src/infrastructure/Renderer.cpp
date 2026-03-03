#include "Renderer.h"
#include "DxLib.h"
#include "core/interface/ILogger.h"

namespace infrastructure
{
	void Renderer::drawModel(int modelHandle, const core::Vector3& position)
	{
		if (modelHandle == -1)
		{
			LOG_E("モデルが読み込まれていません");
			return;
		}

		VECTOR pos = { position.x, position.y, position.z };
		MV1SetPosition(modelHandle, pos);
		MV1DrawModel(modelHandle);
	}
}