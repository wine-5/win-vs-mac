#include "CameraSystem.h"
#include "game/component/CameraComponent.h"
#include "game/component/TransformComponent.h"
#include "game/component/CameraEffectComponent.h"
#include <cmath>
#include <algorithm>

namespace game::system
{
	CameraSystem::CameraSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId targetEntityId,
	    core::iface::IInputProvider& inputProvider,
	    core::iface::ICamera& camera)
	    : m_componentManager{ componentManager }
	    , m_targetEntityId{ targetEntityId }
	    , m_inputProvider{ inputProvider }
	    , m_camera{ camera }
	{
	}

	void CameraSystem::update(float deltaTime)
	{
		if (!m_componentManager.has<component::CameraComponent>(m_targetEntityId))
			return;

		auto& camera{ m_componentManager.get<component::CameraComponent>(m_targetEntityId) };
		auto& transform{ m_componentManager.get<component::TransformComponent>(m_targetEntityId) };

		// マウス移動量で yaw/pitch を更新する
		int deltaX{}, deltaY{};
		m_inputProvider.getMouseDelta(deltaX, deltaY);
		camera.m_yaw += deltaX * camera.m_sensitivity;
		camera.m_pitch += deltaY * camera.m_sensitivity;

		// ピッチを可動範囲に制限する
		camera.m_pitch = std::clamp(camera.m_pitch, camera.m_pitchMin, camera.m_pitchMax);

		// カメラ演出（Zoom/ルーズ/Shake）の合成値を取り込む。無ければ無効値（等倍・揺れなし）。
		float fovScale{ 1.0f };
		float distanceScale{ 1.0f };
		core::Vector3 shakeOffset{ 0.0f, 0.0f, 0.0f };
		if (m_componentManager.has<component::CameraEffectComponent>(m_targetEntityId))
		{
			const auto& effect{ m_componentManager.get<component::CameraEffectComponent>(m_targetEntityId) };
			fovScale = effect.m_fovScale;
			distanceScale = effect.m_distanceScale;
			shakeOffset = effect.m_shakeOffset;
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
} // namespace game::system
