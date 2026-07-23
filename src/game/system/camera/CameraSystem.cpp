#include "CameraSystem.h"
#include "game/component/CameraComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/CameraEffectComponent.h"
#include "game/GameManager.h"
#include <cmath>
#include <algorithm>

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

	void CameraSystem::update(float deltaTime)
	{
		// DEBUG: デバッグモード中はDebugCameraSystemがカメラを制御するため通常追従を止める（リリース時に削除）
		if (m_gameManager.isDebugMode())
			return;

		if (!m_componentManager.has<component::CameraComponent>(m_targetEntityId))
			return;

		auto& camera{ m_componentManager.get<component::CameraComponent>(m_targetEntityId) };
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
		if (m_componentManager.has<component::CameraEffectComponent>(m_targetEntityId))
		{
			const auto& effect{ m_componentManager.get<component::CameraEffectComponent>(m_targetEntityId) };
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

		// カメラから注視点への前方向（水平はyaw、垂直はpitch）
		// カメラは注視点の後方 distance に位置する
		core::Vector3 cameraPos{
			lookTarget.x - sinYaw * cosPitch * distance,
			lookTarget.y + sinPitch * distance,
			lookTarget.z - cosYaw * cosPitch * distance
		};

		// シネマ演出（ボス覚醒など）：通常カメラから注視先へ寄ったカメラへブレンドする。
		// 注視点は注視先へ、カメラ位置は「注視先の手前 CINEMATIC_DISTANCE」へそれぞれ補間する
		if (cinematicBlend > 0.0f)
		{
			constexpr float CINEMATIC_DISTANCE{ 420.0f }; // 注視先からカメラまでの距離（寄りの強さ）

			// カメラ位置→注視先の方向（正規化）。ゼロ距離なら方向は作れないため寄りをスキップする
			core::Vector3 toTarget{
				cinematicTarget.x - cameraPos.x,
				cinematicTarget.y - cameraPos.y,
				cinematicTarget.z - cameraPos.z
			};
			const float toTargetLen{ std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z) };
			if (toTargetLen > 0.0f)
			{
				toTarget.x /= toTargetLen;
				toTarget.y /= toTargetLen;
				toTarget.z /= toTargetLen;

				// 寄り切ったときのカメラ位置＝注視先の手前（今のカメラ方向から寄る）
				const core::Vector3 closePos{
					cinematicTarget.x - toTarget.x * CINEMATIC_DISTANCE,
					cinematicTarget.y - toTarget.y * CINEMATIC_DISTANCE,
					cinematicTarget.z - toTarget.z * CINEMATIC_DISTANCE
				};

				cameraPos.x += (closePos.x - cameraPos.x) * cinematicBlend;
				cameraPos.y += (closePos.y - cameraPos.y) * cinematicBlend;
				cameraPos.z += (closePos.z - cameraPos.z) * cinematicBlend;
				lookTarget.x += (cinematicTarget.x - lookTarget.x) * cinematicBlend;
				lookTarget.y += (cinematicTarget.y - lookTarget.y) * cinematicBlend;
				lookTarget.z += (cinematicTarget.z - lookTarget.z) * cinematicBlend;
			}
		}

		// カメラが地面を突き抜けないよう、床より少し上に制限する
		// （下から地面を見ると片面ポリゴンが裏面カリングで消えて真っ黒になるのを防ぐ）
		constexpr float MIN_CAMERA_HEIGHT{ 30.0f };
		if (cameraPos.y < MIN_CAMERA_HEIGHT)
			cameraPos.y = MIN_CAMERA_HEIGHT;

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
