#include "EnemyFactory.h"
#include <algorithm>

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

	core::ecs::EntityId EnemyFactory::create(int modelHandle, const data::EnemyData& enemyData)
	{
		// 敵種ごとの分岐は廃止。中身（アニメ・AI振る舞い）はEnemyDataのレシピが決める
		auto enemy{ std::make_unique<actor::EnemyBase>(
			m_entityManager, m_componentManager, m_resourceManager, modelHandle, enemyData) };

		// 2段階初期化：生成後にinitialize()でコンポーネントを構築する
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

	void EnemyFactory::remove(core::ecs::EntityId id)
	{
		auto idIt{ std::ranges::find(m_enemyIds, id) };
		if (idIt != m_enemyIds.end())
			m_enemyIds.erase(idIt);

		auto enemyIt{ std::ranges::find_if(m_enemies,
			[id](const auto& enemy) { return enemy->getId() == id; }) };
		if (enemyIt != m_enemies.end())
			m_enemies.erase(enemyIt);
	}
} // namespace game::factory