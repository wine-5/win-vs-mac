#include "Renderer.h"
#include "DxLib.h"
#include "core/interface/ILogger.h"

namespace infrastructure
{
	void Renderer::drawModel(int modelHandle, const core::Vector3& position, const core::Vector3& rotation, const core::Vector3& scale)
	{
		if (modelHandle == -1)
		{
			LOG_E("モデルが読み込まれていません");
			return;
		}

		VECTOR pos = { position.x, position.y, position.z };
		VECTOR rot = { rotation.x, rotation.y, rotation.z };
		VECTOR scl = { scale.x, scale.y, scale.z };

		MV1SetScale(modelHandle, scl);
		MV1SetPosition(modelHandle, pos);
		MV1SetRotationXYZ(modelHandle, rot);
		MV1DrawModel(modelHandle);
	}

	void Renderer::drawCollider(const core::Vector3& center, const core::Vector3& size, unsigned int color)
	{
		// コライダーの最小・最大座標を計算
		core::Vector3 min = {
			center.x - size.x / 2.0f,
			center.y - size.y / 2.0f,
			center.z - size.z / 2.0f
		};
		core::Vector3 max = {
			center.x + size.x / 2.0f,
			center.y + size.y / 2.0f,
			center.z + size.z / 2.0f
		};

		VECTOR v1 = VGet(min.x, min.y, min.z);
		VECTOR v2 = VGet(max.x, max.y, max.z);

		// ワイヤーフレームで描画（塗りつぶしなし）
		DrawCube3D(v1, v2, color, color, FALSE);
	}
}