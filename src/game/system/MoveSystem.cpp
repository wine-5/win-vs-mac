#include "MoveSystem.h"
#include "game/component/InputComponent.h"
#include "game/component/VelocityComponent.h"

namespace game::ecs::system
{
    MoveSystem::MoveSystem(ComponentManager& componentManager, EntityId playerId, float moveSpeed)
        : m_componentManager(componentManager)
        , m_playerId(playerId)
        , m_moveSpeed(moveSpeed)
    {
    }

    void MoveSystem::update(float deltaTime)
    {
        auto& input = m_componentManager.get<component::InputComponent>(m_playerId);
        auto& velocity = m_componentManager.get<component::VelocityComponent>(m_playerId);

        velocity.m_velocity.x = input.m_moveX * m_moveSpeed;
        velocity.m_velocity.z = input.m_moveZ * m_moveSpeed;
    }
}