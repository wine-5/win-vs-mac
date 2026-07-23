#include "Renderer.h"
#include "DxLib.h"
#include "core/interface/ILogger.h"
#include <algorithm>
#include <cmath>
#include <numbers>

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

	void Renderer::drawGroundCircle(const core::Vector3& center, float radius, unsigned int color, bool filled)
	{
		if (radius <= 0.0f)
			return;

		constexpr int SEGMENTS{ 48 };
		constexpr float TWO_PI{ 2.0f * std::numbers::pi_v<float> };

		// ARGBのアルファを取り出して半透明合成する（3D描画はRGBのみなのでアルファはブレンドで表現）
		const int alpha{ static_cast<int>((color >> 24) & 0xFF) };
		const int r{ static_cast<int>((color >> 16) & 0xFF) };
		const int g{ static_cast<int>((color >> 8) & 0xFF) };
		const int b{ static_cast<int>(color & 0xFF) };
		const unsigned int drawColor{ GetColor(r, g, b) };

		// 半透明で重ね描きし、地面や敵を隠さないようZバッファへは書き込まない
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		SetWriteZBuffer3D(FALSE);

		const VECTOR c{ VGet(center.x, center.y, center.z) };
		VECTOR prev{ VGet(center.x + radius, center.y, center.z) };
		for (int i{ 1 }; i <= SEGMENTS; ++i)
		{
			const float angle{ TWO_PI * i / SEGMENTS };
			const VECTOR cur{ VGet(center.x + std::cos(angle) * radius, center.y, center.z + std::sin(angle) * radius) };
			if (filled)
				DrawTriangle3D(c, prev, cur, drawColor, TRUE);
			else
				DrawLine3D(prev, cur, drawColor);
			prev = cur;
		}

		SetWriteZBuffer3D(TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
	}

	void Renderer::drawGroundSector(const core::Vector3& center, float facingRad, float radius,
	    float halfAngleRad, unsigned int color, bool filled)
	{
		if (radius <= 0.0f || halfAngleRad <= 0.0f)
			return;

		constexpr int SEGMENTS{ 48 };

		// ARGBのアルファを取り出して半透明合成する（円と同じ扱い）
		const int alpha{ static_cast<int>((color >> 24) & 0xFF) };
		const int r{ static_cast<int>((color >> 16) & 0xFF) };
		const int g{ static_cast<int>((color >> 8) & 0xFF) };
		const int b{ static_cast<int>(color & 0xFF) };
		const unsigned int drawColor{ GetColor(r, g, b) };

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		SetWriteZBuffer3D(FALSE);

		const VECTOR c{ VGet(center.x, center.y, center.z) };
		const float start{ facingRad - halfAngleRad };
		const float sweep{ 2.0f * halfAngleRad };

		VECTOR prev{ VGet(center.x + std::cos(start) * radius, center.y, center.z + std::sin(start) * radius) };
		for (int i{ 1 }; i <= SEGMENTS; ++i)
		{
			const float angle{ start + sweep * i / SEGMENTS };
			const VECTOR cur{ VGet(center.x + std::cos(angle) * radius, center.y, center.z + std::sin(angle) * radius) };
			if (filled)
				DrawTriangle3D(c, prev, cur, drawColor, TRUE);
			else
				DrawLine3D(prev, cur, drawColor);
			prev = cur;
		}

		// 輪郭のみの場合は要から両端への2辺も描いて扇の形を閉じる
		if (!filled)
		{
			const VECTOR endA{ VGet(center.x + std::cos(start) * radius, center.y, center.z + std::sin(start) * radius) };
			const VECTOR endB{ VGet(center.x + std::cos(start + sweep) * radius, center.y, center.z + std::sin(start + sweep) * radius) };
			DrawLine3D(c, endA, drawColor);
			DrawLine3D(c, endB, drawColor);
		}

		SetWriteZBuffer3D(TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
	}

	void Renderer::drawSpinningModelFacing(int modelHandle, const core::Vector3& position,
	    const core::Vector3& scale, const core::Vector3& centerOffset,
	    const core::Vector3& faceDir, float spinAngle)
	{
		if (modelHandle == -1)
			return;

		const VECTOR pos{ VGet(position.x, position.y, position.z) };

		// モデルの正面(ローカル+Z)を faceDir へ向ける
		VECTOR forward{ VGet(faceDir.x, faceDir.y, faceDir.z) };
		if ((forward.x * forward.x + forward.y * forward.y + forward.z * forward.z) <= 0.0001f)
			forward = VGet(0.0f, 0.0f, -1.0f);
		forward = VNorm(forward);

		// 面の基準上方向（forwardがほぼ垂直なら別軸を使い退化を避ける）
		VECTOR upRef{ (std::abs(forward.y) > 0.99f) ? VGet(0.0f, 0.0f, 1.0f) : VGet(0.0f, 1.0f, 0.0f) };
		VECTOR right{ VNorm(VCross(upRef, forward)) };
		VECTOR up{ VCross(forward, right) };

		// forward軸まわりに面内スピンさせる
		const float c{ std::cos(spinAngle) };
		const float s{ std::sin(spinAngle) };
		VECTOR xAxis{ VScale(VAdd(VScale(right, c), VScale(up, s)), scale.x) };
		VECTOR yAxis{ VScale(VAdd(VScale(right, -s), VScale(up, c)), scale.y) };
		VECTOR zAxis{ VScale(forward, scale.z) };

		// 見た目中心(centerOffset)をpositionへ合わせる平行移動（原点まわりの円運動を打ち消す）
		const VECTOR centerWorld{ VAdd(VAdd(VScale(xAxis, centerOffset.x), VScale(yAxis, centerOffset.y)), VScale(zAxis, centerOffset.z)) };
		const VECTOR translation{ VSub(pos, centerWorld) };

		// ローカル→ワールド行列（DxLibは行ベクトル規約：各行がローカル基底の像）
		MATRIX m{ MGetIdent() };
		m.m[0][0] = xAxis.x;
		m.m[0][1] = xAxis.y;
		m.m[0][2] = xAxis.z;
		m.m[1][0] = yAxis.x;
		m.m[1][1] = yAxis.y;
		m.m[1][2] = yAxis.z;
		m.m[2][0] = zAxis.x;
		m.m[2][1] = zAxis.y;
		m.m[2][2] = zAxis.z;
		m.m[3][0] = translation.x;
		m.m[3][1] = translation.y;
		m.m[3][2] = translation.z;

		MV1SetMatrix(modelHandle, m);
		MV1DrawModel(modelHandle);
	}

	core::Vector3 Renderer::worldToScreen(const core::Vector3& worldPos)
	{
		VECTOR screen = ConvWorldPosToScreenPos(VGet(worldPos.x, worldPos.y, worldPos.z));
		return { screen.x, screen.y, screen.z };
	}
} // namespace infrastructure