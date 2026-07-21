#include "EnemyDeathSystem.h"
#include "game/component/DeathComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/ai/MeleeChaseAIComponent.h"
#include "game/component/ai/RangeKeepAIComponent.h"
#include "game/component/ai/BossAIComponent.h"
#include "game/constant/EnemyType.h"

namespace
{
	/**
	 * @brief AI専用マーカーコンポーネントの有無からEnemyTypeを判定する
	 * @param componentManager ComponentManagerの参照
	 * @param entityId 判定対象のEntityId
	 * @return 対応するEnemyType
	 */
	game::constant::EnemyType inferEnemyType(core::ecs::ComponentManager& componentManager, core::ecs::EntityId entityId)
	{
		using game::constant::EnemyType;
		if (componentManager.has<game::component::ai::BossAIComponent>(entityId))
			return EnemyType::Mac;
		if (componentManager.has<game::component::ai::RangeKeepAIComponent>(entityId))
			return EnemyType::Safari;
		return EnemyType::Xcode;
	}
} // namespace

namespace game::system
{
	EnemyDeathSystem::EnemyDeathSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityManager& entityManager,
	    core::base::EventBus& eventBus,
	    game::factory::EnemySpawner& enemySpawner)
	    : m_componentManager{ componentManager }
	    , m_entityManager{ entityManager }
	    , m_eventBus{ eventBus }
	    , m_enemySpawner{ enemySpawner }
	{
		m_eventBus.subscribe<event::EnemyDeadEvent>(
		    [this](const event::EnemyDeadEvent& e)
		    { onEnemyDead(e); });
	}

	void EnemyDeathSystem::onEnemyDead(const event::EnemyDeadEvent& e)
	{
		if (!m_componentManager.has<component::DeathComponent>(e.m_entityId))
			m_componentManager.add<component::DeathComponent>(e.m_entityId, component::DeathComponent{});
	}

	void EnemyDeathSystem::update(float deltaTime)
	{
		auto entities{ m_componentManager.getAllEntities<component::DeathComponent>() };
		for (auto entityId : entities)
		{
			auto& death{ m_componentManager.get<component::DeathComponent>(entityId) };
			death.m_timer -= deltaTime;
			if (death.m_timer > 0.0f)
				continue;

			const auto type{ inferEnemyType(m_componentManager, entityId) };
			const int modelHandle{ m_componentManager.has<component::RenderComponent>(entityId)
				                       ? m_componentManager.get<component::RenderComponent>(entityId).m_modelHandle
				                       : -1 };

			m_enemySpawner.returnEnemy(type, entityId, modelHandle);
			m_componentManager.removeAll(entityId);
			m_entityManager.destroy(core::ecs::Entity(entityId));
		}
	}
} // namespace game::system
