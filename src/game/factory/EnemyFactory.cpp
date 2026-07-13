#include "EnemyFactory.h"
#include "game/actor/XcodeEnemy.h"
#include "game/actor/SafariEnemy.h"
#include "game/actor/MacEnemy.h"

namespace game::factory
{
	EnemyFactory::EnemyFactory(
		core::ecs::EntityManager& entityManager,
		core::ecs::ComponentManager& componentManager,
		core::iface::IResourceManager& resourceManager)
		: m_entityManager{ entityManager }
		, m_componentManager{ componentManager }
		, m_resourceManager{ resourceManager }
	{
	}

	core::ecs::EntityId EnemyFactory::create(constant::EnemyType type, int modelHandle, const data::EnemyData& enemyData)
	{
		std::unique_ptr<actor::EnemyBase> enemy{};

		switch (type)
		{
		case game::constant::EnemyType::Xcode:
			enemy = std::make_unique<actor::XcodeEnemy>(
				m_entityManager, m_componentManager, m_resourceManager, modelHandle, enemyData);
			break;
		case game::constant::EnemyType::Safari:
			enemy = std::make_unique<actor::SafariEnemy>(
				m_entityManager, m_componentManager, m_resourceManager, modelHandle, enemyData);
			break;
		case game::constant::EnemyType::Mac:
			enemy = std::make_unique<actor::MacEnemy>(
				m_entityManager, m_componentManager, m_resourceManager, modelHandle, enemyData);
			break;
		}

		// 2段階初期化：生成後に呼ぶことで仮想フックが派生クラスに正しく届く
		enemy->initialize();

		core::ecs::EntityId id{ enemy->getId() };
		m_enemies.push_back(std::move(enemy));
		m_enemyIds.push_back(id);
		return id;
	}

	const std::vector<core::ecs::EntityId>& EnemyFactory::getEnemyIds() const noexcept
	{
		return m_enemyIds;
	}
}