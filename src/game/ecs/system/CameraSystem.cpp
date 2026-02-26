#include "CameraSystem.h"
#include <DxLib.h>
#include "game/ecs/component/TransformComponent.h"
#include "game/ecs/component/CameraComponent.h"

namespace game::ecs::system
{
	CameraSystem::CameraSystem(ComponentManager& componentManager, EntityId playerId)
		: m_componentManager(componentManager)
		, m_playerId(playerId)
	{
	}

    void CameraSystem::update(float deltaTime)
    {
        auto& transform = m_componentManager.get<component::TransformComponent>(m_playerId);
        auto& camera = m_componentManager.get<component::CameraComponent>(m_playerId);

        // カメラ位置 = Playerの位置 + オフセット
        camera.m_position.x = transform.m_position.x + camera.m_offset.x;
        camera.m_position.y = transform.m_position.y + camera.m_offset.y;
        camera.m_position.z = transform.m_position.z + camera.m_offset.z;

        // 注視点 = Playerの位置
        camera.m_target = transform.m_position;

        // DxLibにカメラ情報を反映
        SetCameraPositionAndTarget_UpVecY(camera.m_position, camera.m_target);
    }
}