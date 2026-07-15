#include "CameraSystem.h"
#include "game/component/CameraComponent.h"
#include "game/component/TransformComponent.h"
#include <cmath>
#include <algorithm>

namespace game::system
{
	CameraSystem::CameraSystem(core::ecs::ComponentManager& componentManager,
	                           core::ecs::EntityId targetEntityId,
	                           core::iface::IInputProvider& inputProvider,
	                           core::iface::ICamera& camera)
	    : m_componentManager{ componentManager }, m_targetEntityId{ targetEntityId }, m_inputProvider{ inputProvider }, m_camera{ camera }
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
			lookTarget.x - sinYaw * cosPitch * camera.m_distance,
			lookTarget.y + sinPitch * camera.m_distance,
			lookTarget.z - cosYaw * cosPitch * camera.m_distance
		};

		m_camera.setLookAt(cameraPos, lookTarget);
	}
} // namespace game::system
