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

	void Renderer::drawDebugSphere(const core::Vector3& center, float radius, unsigned int color)
	{
		constexpr int DIV_NUM{ 16 }; // 球の分割数（デバッグ用なので粗くてよい）
		VECTOR pos = VGet(center.x, center.y, center.z);

		// ワイヤーフレームで描画（塗りつぶしなし）
		DrawSphere3D(pos, radius, DIV_NUM, color, color, FALSE);
	}

	void Renderer::drawDebugCapsule(const core::Vector3& bottom, const core::Vector3& top, float radius, unsigned int color)
	{
		constexpr int DIV_NUM{ 16 }; // カプセルの分割数（デバッグ用なので粗くてよい）
		VECTOR pos1 = VGet(bottom.x, bottom.y, bottom.z);
		VECTOR pos2 = VGet(top.x, top.y, top.z);

		// ワイヤーフレームで描画（塗りつぶしなし）
		DrawCapsule3D(pos1, pos2, radius, DIV_NUM, color, color, FALSE);
	}

	void Renderer::drawBillboard(int imageHandle, const core::Vector3& position, float size)
	{
		if (imageHandle == -1)
			return;

		VECTOR pos = VGet(position.x, position.y, position.z);
		// 中心(0.5, 0.5)基準・回転なし・透過有効で描画する
		DrawBillboard3D(pos, 0.5f, 0.5f, size, 0.0f, imageHandle, TRUE);
	}
} // namespace infrastructure