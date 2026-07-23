#include "DebugCameraSystem.h"
#include "game/GameManager.h"
#include "game/PauseManager.h"
#include "game/component/CameraComponent.h"
#include "game/component/TransformComponent.h"
#include <cmath>
#include <algorithm>

// DEBUG: このファイルはデバッグ用のフリーカメラ機能。リリース時にまとめて削除する。

namespace game::system::camera
{
	namespace
	{
		constexpr float MOVE_SPEED{ 700.0f };  // フリーカメラの移動速度（u/秒）
		constexpr float SENSITIVITY{ 0.003f }; // マウス感度（ラジアン/ピクセル）
		constexpr float PITCH_LIMIT{ 1.5f };   // ピッチの可動範囲（真上/真下でのフリップ防止）
	} // namespace

	DebugCameraSystem::DebugCameraSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId targetEntityId,
	    core::iface::IInputProvider& inputProvider,
	    core::iface::ICamera& camera,
	    GameManager& gameManager,
	    PauseManager& pauseManager)
	    : m_componentManager{ componentManager }
	    , m_targetEntityId{ targetEntityId }
	    , m_inputProvider{ inputProvider }
	    , m_camera{ camera }
	    , m_gameManager{ gameManager }
	    , m_pauseManager{ pauseManager }
	{
	}

	void DebugCameraSystem::update(float deltaTime)
	{
		// F1のフリーカメラ中、またはF2のシーンビュー（時間停止）中に動作する
		const bool debugMode{ m_gameManager.isDebugMode() ||
			                  m_pauseManager.isPausedBy(PauseReason::DebugSceneView) };

		// デバッグOFF時は何もしない（通常のCameraSystemに任せる）
		if (!debugMode)
		{
			m_wasDebugMode = false;
			return;
		}

		// デバッグモードに入った瞬間は、通常カメラの位置・向きを引き継いで違和感なく開始する
		if (!m_wasDebugMode)
		{
			initializeFromOrbitCamera();
			m_wasDebugMode = true;
		}

		// マウス移動量で視点（yaw/pitch）を更新する
		int deltaX{}, deltaY{};
		m_inputProvider.getMouseDelta(deltaX, deltaY);
		m_yaw += deltaX * SENSITIVITY;
		m_pitch += deltaY * SENSITIVITY;
		m_pitch = std::clamp(m_pitch, -PITCH_LIMIT, PITCH_LIMIT);

		const float cosPitch{ std::cos(m_pitch) };
		const float sinPitch{ std::sin(m_pitch) };
		const float sinYaw{ std::sin(m_yaw) };
		const float cosYaw{ std::cos(m_yaw) };

		// WASDでフリーカメラを操作する（Playerは矢印キーが担当）。
		// 水平前方向（yawのみ）と右方向。上下移動はワールドY軸で行う。
		const core::Vector3 forward{ sinYaw, 0.0f, cosYaw };
		const core::Vector3 right{ cosYaw, 0.0f, -sinYaw };

		float moveForward{ 0.0f };
		float moveRight{ 0.0f };
		float moveUp{ 0.0f };
		if (m_inputProvider.isKeyDown(core::input::KeyCode::W))
			moveForward += 1.0f;
		if (m_inputProvider.isKeyDown(core::input::KeyCode::S))
			moveForward -= 1.0f;
		if (m_inputProvider.isKeyDown(core::input::KeyCode::D))
			moveRight += 1.0f;
		if (m_inputProvider.isKeyDown(core::input::KeyCode::A))
			moveRight -= 1.0f;
		if (m_inputProvider.isKeyDown(core::input::KeyCode::Space))
			moveUp += 1.0f;
		if (m_inputProvider.isKeyDown(core::input::KeyCode::Shift))
			moveUp -= 1.0f;

		const float step{ MOVE_SPEED * deltaTime };
		m_position.x += (forward.x * moveForward + right.x * moveRight) * step;
		m_position.z += (forward.z * moveForward + right.z * moveRight) * step;
		m_position.y += moveUp * step;

		// Playerの移動基準yawをカメラの向きに合わせる（矢印キー移動をカメラ視点基準にする）
		if (m_componentManager.has<component::CameraComponent>(m_targetEntityId))
			m_componentManager.get<component::CameraComponent>(m_targetEntityId).m_yaw = m_yaw;

		// 視線方向（pitchを含む3D前方向）から注視点を求めてカメラへ渡す
		const core::Vector3 lookForward{ sinYaw * cosPitch, -sinPitch, cosYaw * cosPitch };
		const core::Vector3 lookTarget{
			m_position.x + lookForward.x,
			m_position.y + lookForward.y,
			m_position.z + lookForward.z
		};
		m_camera.setLookAt(m_position, lookTarget);
	}

	void DebugCameraSystem::initializeFromOrbitCamera()
	{
		if (!m_componentManager.has<component::CameraComponent>(m_targetEntityId) ||
		    !m_componentManager.has<component::TransformComponent>(m_targetEntityId))
			return;

		const auto& camera{ m_componentManager.get<component::CameraComponent>(m_targetEntityId) };
		const auto& transform{ m_componentManager.get<component::TransformComponent>(m_targetEntityId) };

		// 通常カメラ（CameraSystem）と同じ計算で現在のカメラ位置・向きを再現する
		m_yaw = camera.m_yaw;
		m_pitch = camera.m_pitch;

		const float cosPitch{ std::cos(m_pitch) };
		const float sinPitch{ std::sin(m_pitch) };
		const float sinYaw{ std::sin(m_yaw) };
		const float cosYaw{ std::cos(m_yaw) };

		const core::Vector3 lookTarget{
			transform.m_position.x,
			transform.m_position.y + camera.m_targetHeight,
			transform.m_position.z
		};

		m_position = core::Vector3{
			lookTarget.x - sinYaw * cosPitch * camera.m_distance,
			lookTarget.y + sinPitch * camera.m_distance,
			lookTarget.z - cosYaw * cosPitch * camera.m_distance
		};
	}
} // namespace game::system::camera
