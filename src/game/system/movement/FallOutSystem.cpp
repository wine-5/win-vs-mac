#include "FallOutSystem.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/movement/FallRecoveryComponent.h"
#include "game/component/TagComponent.h"
#include "game/constant/Tag.h"

namespace
{
	// 最後に立っていた場所からこれ以上下がったら、奈落へ落ちたとみなす。
	// 坂や段差による正規の落差より十分大きくとる
	constexpr float FALL_LIMIT{ 1200.0f };
} // namespace

namespace game::system::movement
{
	FallOutSystem::FallOutSystem(core::ecs::ComponentManager& componentManager, core::base::EventBus& eventBus)
	    : m_componentManager{ componentManager }
	    , m_eventBus{ eventBus }
	{
	}

	bool FallOutSystem::hasFallenOut(core::ecs::EntityId entityId) const
	{
		const auto* recovery{ m_componentManager.tryGet<component::movement::FallRecoveryComponent>(entityId) };
		if (recovery == nullptr || !recovery->m_hasSafePosition)
			return false;

		// 深く潜るステージなので、絶対的な高さではなく「最後の足場からの落差」で見る
		const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(entityId) };
		return recovery->m_lastSafePosition.y - transform.m_position.y >= FALL_LIMIT;
	}

	void FallOutSystem::punishPlayer(core::ecs::EntityId entityId)
	{
		auto& transform{ m_componentManager.get<component::movement::TransformComponent>(entityId) };
		auto& velocity{ m_componentManager.get<component::movement::VelocityComponent>(entityId) };
		const auto& recovery{ m_componentManager.get<component::movement::FallRecoveryComponent>(entityId) };

		transform.m_position = recovery.m_lastSafePosition;
		velocity.m_velocity = core::Vector3{};
		velocity.m_externalVelocity = core::Vector3{};
	}

	void FallOutSystem::update(float /*deltaTime*/)
	{
		const auto entities{ m_componentManager.getAllEntities<component::movement::FallRecoveryComponent>() };
		for (const auto entityId : entities)
		{
			if (!hasFallenOut(entityId))
				continue;

			const auto* tag{ m_componentManager.tryGet<component::TagComponent>(entityId) };
			if (tag == nullptr)
				continue;

			if (tag->m_tag == constant::Tag::Player)
				punishPlayer(entityId);
		}
	}
} // namespace game::system::movement
