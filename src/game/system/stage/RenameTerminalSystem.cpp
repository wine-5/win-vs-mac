#include "RenameTerminalSystem.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/stage/RenameTerminalComponent.h"

namespace game::system::stage
{
	RenameTerminalSystem::RenameTerminalSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId playerId)
	    : m_componentManager{ componentManager }
	    , m_playerId{ playerId }
	{
	}

	void RenameTerminalSystem::update([[maybe_unused]] float deltaTime)
	{
		m_nearTerminalId = core::ecs::INVALID_ENTITY_ID;

		const auto* playerTransform{ m_componentManager.tryGet<component::movement::TransformComponent>(m_playerId) };
		if (playerTransform == nullptr)
			return;

		float nearestSq{ 0.0f };

		const auto terminals{ m_componentManager.getAllEntities<component::stage::RenameTerminalComponent>() };
		for (const auto id : terminals)
		{
			auto& terminal{ m_componentManager.get<component::stage::RenameTerminalComponent>(id) };
			terminal.m_isPlayerNear = false;

			const auto* transform{ m_componentManager.tryGet<component::movement::TransformComponent>(id) };
			if (transform == nullptr)
				continue;

			// 高さは見ない。端末の上へ乗ったり段差の下から近づいたりしても
			// 「そばにいる」と判断できるようにする
			const float dx{ playerTransform->m_position.x - transform->m_position.x };
			const float dz{ playerTransform->m_position.z - transform->m_position.z };
			const float distanceSq{ dx * dx + dz * dz };
			if (distanceSq > terminal.m_interactRange * terminal.m_interactRange)
				continue;

			// 一番近いものだけを残す。近接して置かれたときに案内が重なって読めなくなる
			if (m_nearTerminalId == core::ecs::INVALID_ENTITY_ID || distanceSq < nearestSq)
			{
				m_nearTerminalId = id;
				nearestSq = distanceSq;
			}
		}

		if (m_nearTerminalId != core::ecs::INVALID_ENTITY_ID)
			m_componentManager.get<component::stage::RenameTerminalComponent>(m_nearTerminalId).m_isPlayerNear = true;
	}

	core::ecs::EntityId RenameTerminalSystem::getNearTerminalId() const noexcept
	{
		return m_nearTerminalId;
	}
} // namespace game::system::stage
