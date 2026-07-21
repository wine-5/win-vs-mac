#include "Renderer.h"
#include "DxLib.h"
#include "core/interface/ILogger.h"
#include <algorithm>

namespace
{
	// ディゾルブ演出で最終的に寄せる赤色（元の色からこの色へブレンドする）
	constexpr float DISSOLVE_RED_R{ 1.0f };
	constexpr float DISSOLVE_RED_G{ 0.1f };
	constexpr float DISSOLVE_RED_B{ 0.08f };
} // namespace

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

	void Renderer::applyDeathDissolve(int modelHandle, float redProgress, float alpha)
	{
		if (modelHandle == -1)
			return;

		redProgress = std::clamp(redProgress, 0.0f, 1.0f);
		alpha = std::clamp(alpha, 0.0f, 1.0f);
		const float progress{ redProgress };

		const int materialNum{ MV1GetMaterialNum(modelHandle) };
		if (materialNum <= 0)
			return;

		// 初回呼び出し時に元の色を保存し、以後はこのプールされたハンドルを
		// 使い回しても正しく元の色から赤へブレンドできるようにする
		auto& cache{ m_originalColors[modelHandle] };
		if (cache.empty())
		{
			cache.reserve(materialNum);
			for (int i{ 0 }; i < materialNum; ++i)
			{
				const COLOR_F dif{ MV1GetMaterialDifColor(modelHandle, i) };
				const COLOR_F emi{ MV1GetMaterialEmiColor(modelHandle, i) };
				cache.push_back({ { dif.r, dif.g, dif.b, dif.a }, { emi.r, emi.g, emi.b, emi.a } });
			}
			MV1SetMaterialDrawBlendModeAll(modelHandle, DX_BLENDMODE_ALPHA);
		}

		for (int i{ 0 }; i < materialNum && i < static_cast<int>(cache.size()); ++i)
		{
			const auto& origDif{ cache[i].m_dif };
			const auto& origEmi{ cache[i].m_emi };

			// ディフューズも赤へ寄せる（ライティングONのマテリアル対策）
			COLOR_F dif{};
			dif.r = origDif[0] + (DISSOLVE_RED_R - origDif[0]) * progress;
			dif.g = origDif[1] + (DISSOLVE_RED_G - origDif[1]) * progress;
			dif.b = origDif[2] + (DISSOLVE_RED_B - origDif[2]) * progress;
			dif.a = origDif[3];
			MV1SetMaterialDifColor(modelHandle, i, dif);

			// このプロジェクトはライティングOFF。ディフューズ色は乗りづらいため、
			// 自己発光（エミッシブ）に赤を足して確実に赤く光らせる
			COLOR_F emi{};
			emi.r = origEmi[0] + (DISSOLVE_RED_R - origEmi[0]) * progress;
			emi.g = origEmi[1] + (DISSOLVE_RED_G - origEmi[1]) * progress;
			emi.b = origEmi[2] + (DISSOLVE_RED_B - origEmi[2]) * progress;
			emi.a = origEmi[3];
			MV1SetMaterialEmiColor(modelHandle, i, emi);
		}

		// ディゾルブ（消失）はアルファのフェードアウトで表現する。
		// フェード開始タイミングの制御は呼び出し側（EnemyDeathSystem）の責務
		const int alphaParam{ static_cast<int>(255.0f * alpha) };
		MV1SetMaterialDrawBlendParamAll(modelHandle, alphaParam);
	}

	void Renderer::resetModelAppearance(int modelHandle)
	{
		auto it{ m_originalColors.find(modelHandle) };
		if (it == m_originalColors.end())
			return;

		const int materialNum{ MV1GetMaterialNum(modelHandle) };
		for (int i{ 0 }; i < materialNum && i < static_cast<int>(it->second.size()); ++i)
		{
			const auto& orig{ it->second[i] };
			MV1SetMaterialDifColor(modelHandle, i, COLOR_F{ orig.m_dif[0], orig.m_dif[1], orig.m_dif[2], orig.m_dif[3] });
			MV1SetMaterialEmiColor(modelHandle, i, COLOR_F{ orig.m_emi[0], orig.m_emi[1], orig.m_emi[2], orig.m_emi[3] });
		}
		MV1SetMaterialDrawBlendModeAll(modelHandle, DX_BLENDMODE_NOBLEND);
		MV1SetMaterialDrawBlendParamAll(modelHandle, 255);

		m_originalColors.erase(it);
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

	core::Vector3 Renderer::worldToScreen(const core::Vector3& worldPos)
	{
		VECTOR screen = ConvWorldPosToScreenPos(VGet(worldPos.x, worldPos.y, worldPos.z));
		return { screen.x, screen.y, screen.z };
	}
} // namespace infrastructure