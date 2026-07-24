#include "CameraSystem.h"
#include "game/component/camera/CameraComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/camera/CameraEffectComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/TagComponent.h"
#include "game/constant/Tag.h"
#include "game/GameManager.h"
#include "core/utility/Rotation.h"
#include <cmath>
#include <algorithm>

namespace
{
	// カメラを壁からこれだけ離す。0にすると壁面に密着して面が消える
	constexpr float WALL_PADDING{ 45.0f };
	// どれだけ寄せてもこれ以上は近づけない（プレイヤーの中に入らないように）
	constexpr float MIN_WALL_DISTANCE{ 90.0f };
	// 壁から離れたあと元の距離へ戻る速さ（毎秒の割合）。寄るときは即座に寄せる
	constexpr float DISTANCE_RESTORE_PER_SEC{ 4.0f };
} // namespace

namespace game::system::camera
{
	CameraSystem::CameraSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId targetEntityId,
	    core::iface::IInputProvider& inputProvider,
	    core::iface::ICamera& camera,
	    GameManager& gameManager)
	    : m_componentManager{ componentManager }
	    , m_targetEntityId{ targetEntityId }
	    , m_inputProvider{ inputProvider }
	    , m_camera{ camera }
	    , m_gameManager{ gameManager }
	{
	}

	float CameraSystem::clampDistanceByWalls(const core::Vector3& lookTarget,
	    float desiredDistance, const core::Vector3& direction) const
	{
		float nearest{ desiredDistance };

		const auto entities{ m_componentManager.getAllEntities<component::combat::ColliderComponent>() };
		for (const auto id : entities)
		{
			// 遮蔽物として見るのはステージの配置物だけ。敵で寄られると戦闘中に画面が暴れる
			const auto* tag{ m_componentManager.tryGet<component::TagComponent>(id) };
			if (tag == nullptr || tag->m_tag != constant::Tag::Ground)
				continue;

			const auto* transform{ m_componentManager.tryGet<component::movement::TransformComponent>(id) };
			if (transform == nullptr)
				continue;

			const auto& collider{ m_componentManager.get<component::combat::ColliderComponent>(id) };
			const core::Vector3 center{ transform->m_position + collider.m_offset };
			const core::Vector3 yaw{ 0.0f, collider.m_rotationY, 0.0f };

			// 配置物の向きに合わせた座標系へ移すと、傾いた壁でも軸並行のスラブ判定で済む
			const core::Vector3 origin{ core::utility::inverseRotateEulerXYZ(lookTarget - center, yaw) };
			const core::Vector3 ray{ core::utility::inverseRotateEulerXYZ(direction, yaw) };
			const core::Vector3 half{ collider.m_size * 0.5f + core::Vector3{ WALL_PADDING, WALL_PADDING, WALL_PADDING } };

			// 各軸のスラブに入っている区間を求め、3軸すべてで重なる区間が交差部分になる
			float entry{ 0.0f };
			float exit{ nearest };
			const float originAxis[3]{ origin.x, origin.y, origin.z };
			const float rayAxis[3]{ ray.x, ray.y, ray.z };
			const float halfAxis[3]{ half.x, half.y, half.z };

			bool missed{ false };
			for (int axis{ 0 }; axis < 3 && !missed; ++axis)
			{
				if (std::abs(rayAxis[axis]) < 1e-6f)
				{
					// この軸に平行。最初からスラブの外なら交差しない
					missed = std::abs(originAxis[axis]) > halfAxis[axis];
					continue;
				}

				const float inverse{ 1.0f / rayAxis[axis] };
				float near{ (-halfAxis[axis] - originAxis[axis]) * inverse };
				float far{ (halfAxis[axis] - originAxis[axis]) * inverse };
				if (near > far)
					std::swap(near, far);

				entry = std::max(entry, near);
				exit = std::min(exit, far);
				missed = entry > exit;
			}

			// 注視点がすでに箱の中（entryが0）なら寄せない。寄せるとカメラがプレイヤーに埋まる
			if (!missed && entry > 0.0f)
				nearest = entry;
		}

		return std::max(nearest, MIN_WALL_DISTANCE);
	}

	void CameraSystem::update(float deltaTime)
	{
		// DEBUG: デバッグモード中はDebugCameraSystemがカメラを制御するため通常追従を止める（リリース時に削除）
		if (m_gameManager.isDebugMode())
			return;

		if (!m_componentManager.has<component::camera::CameraComponent>(m_targetEntityId))
			return;

		auto& camera{ m_componentManager.get<component::camera::CameraComponent>(m_targetEntityId) };
		auto& transform{ m_componentManager.get<component::movement::TransformComponent>(m_targetEntityId) };

		// マウス移動量で yaw/pitch を更新する
		int deltaX{}, deltaY{};
		m_inputProvider.getMouseDelta(deltaX, deltaY);
		camera.m_yaw += deltaX * camera.m_sensitivity;
		camera.m_pitch += deltaY * camera.m_sensitivity;

		// ピッチを可動範囲に制限する
		camera.m_pitch = std::clamp(camera.m_pitch, camera.m_pitchMin, camera.m_pitchMax);

		// カメラ演出（Zoom/ルーズ/Shake/シネマ）の合成値を取り込む。無ければ無効値（等倍・揺れなし）。
		float fovScale{ 1.0f };
		float distanceScale{ 1.0f };
		core::Vector3 shakeOffset{ 0.0f, 0.0f, 0.0f };
		float cinematicBlend{ 0.0f };
		core::Vector3 cinematicTarget{ 0.0f, 0.0f, 0.0f };
		if (m_componentManager.has<component::camera::CameraEffectComponent>(m_targetEntityId))
		{
			const auto& effect{ m_componentManager.get<component::camera::CameraEffectComponent>(m_targetEntityId) };
			fovScale = effect.m_fovScale;
			distanceScale = effect.m_distanceScale;
			// 揺れは通常演出（被弾）とボス覚醒演出を加算する
			shakeOffset = effect.m_shakeOffset + effect.m_awakenShakeOffset;
			cinematicBlend = effect.m_cinematicBlend;
			cinematicTarget = effect.m_cinematicTarget;
		}

		// 演出のルーズ倍率を反映した実効距離
		const float distance{ camera.m_distance * distanceScale };

		// 注視点はプレイヤーの少し上（頭あたり）
		core::Vector3 lookTarget{
			transform.m_position.x,
			transform.m_position.y + camera.m_targetHeight,
			transform.m_position.z
		};

		// yaw/pitch/距離からカメラの位置を計算する
		const float cosPitch{ std::cos(camera.m_pitch) };
		const float sinPitch{ std::sin(camera.m_pitch) };
		const float sinYaw{ std::sin(camera.m_yaw) };
		const float cosYaw{ std::cos(camera.m_yaw) };

		// 注視点からカメラへ向かう単位ベクトル（水平はyaw、垂直はpitch）
		const core::Vector3 toCamera{ -sinYaw * cosPitch, sinPitch, -cosYaw * cosPitch };

		// 壁に遮られるならその手前まで寄せる。
		// 寄るのは即座（遮られた瞬間に見えないと困る）、離れて戻るのは緩やかにして画面が跳ねないようにする
		const float wallDistance{ clampDistanceByWalls(lookTarget, distance, toCamera) };
		if (m_currentDistance <= 0.0f || wallDistance < m_currentDistance)
			m_currentDistance = wallDistance;
		else
			m_currentDistance += (wallDistance - m_currentDistance) *
			                     std::min(DISTANCE_RESTORE_PER_SEC * deltaTime, 1.0f);

		core::Vector3 cameraPos{
			lookTarget.x + toCamera.x * m_currentDistance,
			lookTarget.y + toCamera.y * m_currentDistance,
			lookTarget.z + toCamera.z * m_currentDistance
		};

		// シネマ演出（ボス覚醒など）：通常カメラから注視先へ寄ったカメラへブレンドする。
		// 注視点は注視先へ、カメラ位置は「注視先の手前 CINEMATIC_DISTANCE」へそれぞれ補間する
		if (cinematicBlend > 0.0f)
		{
			constexpr float CINEMATIC_DISTANCE{ 420.0f }; // 注視先からカメラまでの距離（寄りの強さ）

			// カメラ位置→注視先の方向（正規化）。ゼロ距離なら方向は作れないため寄りをスキップする
			const core::Vector3 toTarget{ cinematicTarget - cameraPos };
			if (toTarget.lengthSq() > 0.0f)
			{
				// 寄り切ったときのカメラ位置＝注視先の手前（今のカメラ方向から寄る）
				const core::Vector3 closePos{ cinematicTarget - toTarget.normalized() * CINEMATIC_DISTANCE };

				cameraPos += (closePos - cameraPos) * cinematicBlend;
				lookTarget += (cinematicTarget - lookTarget) * cinematicBlend;
			}
		}

		// カメラが地面を突き抜けないよう、床より少し上に制限する
		// （下から地面を見ると片面ポリゴンが裏面カリングで消えて真っ黒になるのを防ぐ）
		// 高さはワールド絶対値ではなくプレイヤーの足元基準で見る。
		// 絶対値にすると坂を下って足元が下がったときにクランプへ張り付き、pitchを動かしても上を向けなくなる。
		constexpr float MIN_CAMERA_HEIGHT{ 30.0f };
		const float minCameraY{ lookTarget.y - camera.m_targetHeight + MIN_CAMERA_HEIGHT };
		if (cameraPos.y < minCameraY)
			cameraPos.y = minCameraY;

		// Shakeの揺れをカメラ位置と注視点の両方へ加算する（画面全体が同じだけ揺れる）
		cameraPos = cameraPos + shakeOffset;
		lookTarget = lookTarget + shakeOffset;

		// 視線方向（単位ベクトル）を保存する。レティクル判定・投射の発射方向に使う。
		camera.m_forward = core::Vector3{ sinYaw * cosPitch, -sinPitch, cosYaw * cosPitch };

		m_camera.setLookAt(cameraPos, lookTarget);
		// FOVにZoom倍率を掛けて視野角を確定する（絞るほど望遠で寄って見える）
		m_camera.setFieldOfView(camera.m_fov * fovScale);
	}
} // namespace game::system::camera
