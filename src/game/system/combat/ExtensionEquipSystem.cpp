#include "ExtensionEquipSystem.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/combat/HealthComponent.h"
#include "game/component/combat/PlayerStatsComponent.h"
#include "game/event/InGameEvents.h"
#include "core/utility/Log.h"
#include <algorithm>

namespace game::system::combat
{
	ExtensionEquipSystem::ExtensionEquipSystem(core::ecs::ComponentManager& componentManager,
	    core::base::EventBus& eventBus,
	    core::iface::IResourceManager& resourceManager,
	    core::ecs::EntityId playerId)
	    : m_componentManager{ componentManager }
	    , m_resourceManager{ resourceManager }
	    , m_playerId{ playerId }
	{
		m_subscriptions.push_back(eventBus.subscribe<event::ExtensionPickedUpEvent>(
		    [this](const event::ExtensionPickedUpEvent& e)
		    {
			    m_pending.push_back(e.m_type);
		    }));
	}

	void ExtensionEquipSystem::update([[maybe_unused]] float deltaTime)
	{
		if (m_pending.empty())
			return;

		for (const auto type : m_pending)
		{
			if (m_equippedCount >= MAX_INGAME_SLOTS)
			{
				// 枠が埋まっている。捨てるか差し替えるかを選ばせるのが本来の設計だが、
				// インベントリが入るまでは黙って持ち越さず捨てる
				core::log::info("拡張子を拾ったが枠が埋まっています（{}/{}）",
				    m_equippedCount, MAX_INGAME_SLOTS);
				continue;
			}

			applyBonus(type);
			++m_equippedCount;
		}
		m_pending.clear();
	}

	int ExtensionEquipSystem::getEquippedCount() const noexcept
	{
		return m_equippedCount;
	}

	void ExtensionEquipSystem::applyBonus(core::data::FileExtensionType type)
	{
		const auto& bonus{ m_resourceManager.getExtensionBonus(type) };

		// 能力値は種類ごとに持ち主が違う（攻撃はAttack、HPはHealth、速度はStats）。
		// PlayerDataへ足しても既に生成済みのコンポーネントには伝わらないため、直接触る
		if (auto* attack{ m_componentManager.tryGet<component::combat::AttackComponent>(m_playerId) })
		{
			attack->m_attackPower += bonus.atk;
			attack->m_attackRange += bonus.attackRange;
			// 発生率は確率なので1.0（必ず出る）を超えないよう頭打ちにする
			attack->m_criticalRate = std::min(attack->m_criticalRate + bonus.criticalRate, 1.0f);
		}

		if (auto* health{ m_componentManager.tryGet<component::combat::HealthComponent>(m_playerId) })
		{
			health->m_maxHp += bonus.hp;
			health->m_defence += bonus.def;

			// 最大HPが増えたぶんはその場で回復させる。増えたのに減ったままだと
			// 「拾って強くなった」感じが出ない
			health->m_currentHp = std::min(health->m_currentHp + bonus.hp, health->m_maxHp);
		}

		if (auto* stats{ m_componentManager.tryGet<component::combat::PlayerStatsComponent>(m_playerId) })
		{
			stats->m_moveSpeed += bonus.spd;
			stats->m_projectileSpeed += bonus.projectileSpeed;
			stats->m_projectileRange += bonus.projectileRange;
		}
	}
} // namespace game::system::combat
