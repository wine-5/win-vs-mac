#include "PlayerStats.h"
#include "core/utility/MathConstants.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/combat/HealthComponent.h"
#include "game/component/combat/PlayerStatsComponent.h"
#include "game/component/combat/PlayerStatBaseComponent.h"

namespace game::utility
{
	PlayerStatValues collectPlayerStats(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId playerId)
	{
		PlayerStatValues stats{};

		if (const auto* attack{ componentManager.tryGet<component::combat::AttackComponent>(playerId) })
		{
			stats[STAT_INDEX_ATK] = attack->m_attackPower;
			stats[STAT_INDEX_RNG] = attack->m_attackRange;
			stats[STAT_INDEX_CRIT] = attack->m_criticalRate * core::utility::RATIO_TO_PERCENT; // 割合を百分率へ
		}
		if (const auto* health{ componentManager.tryGet<component::combat::HealthComponent>(playerId) })
		{
			stats[STAT_INDEX_HP] = health->m_maxHp;
			stats[STAT_INDEX_DEF] = health->m_defence;
		}
		if (const auto* player{ componentManager.tryGet<component::combat::PlayerStatsComponent>(playerId) })
		{
			stats[STAT_INDEX_SPD] = player->m_moveSpeed;
			stats[STAT_INDEX_BSPD] = player->m_projectileSpeed;
			stats[STAT_INDEX_BRNG] = player->m_projectileRange;
		}
		return stats;
	}

	PlayerStatValues collectPlayerBaseStats(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId playerId)
	{
		PlayerStatValues stats{};

		const auto* base{ componentManager.tryGet<component::combat::PlayerStatBaseComponent>(playerId) };
		if (base == nullptr)
			return stats; // 控えが無ければ強化なし扱い（全項目が素の色になる）

		stats[STAT_INDEX_HP] = base->m_maxHp;
		stats[STAT_INDEX_ATK] = base->m_attackPower;
		stats[STAT_INDEX_DEF] = base->m_defence;
		stats[STAT_INDEX_SPD] = base->m_moveSpeed;
		stats[STAT_INDEX_RNG] = base->m_attackRange;
		stats[STAT_INDEX_CRIT] = base->m_criticalRate * core::utility::RATIO_TO_PERCENT; // 現在値と同じ百分率へ揃える
		stats[STAT_INDEX_BSPD] = base->m_projectileSpeed;
		stats[STAT_INDEX_BRNG] = base->m_projectileRange;
		return stats;
	}
} // namespace game::utility
