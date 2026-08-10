#include "ExtensionEquipSystem.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/combat/HealthComponent.h"
#include "game/component/combat/PlayerStatsComponent.h"
#include "game/component/combat/ExtensionInventoryComponent.h"
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
	    , m_eventBus{ eventBus }
	    , m_resourceManager{ resourceManager }
	    , m_playerId{ playerId }
	{
		m_subscriptions.push_back(eventBus.subscribe<event::ExtensionPickedUpEvent>(
		    [this](const event::ExtensionPickedUpEvent& e)
		    {
			    m_pending.push_back(e.m_type);
		    }));

		// 拾ったぶんと違い、入れ替えはその場で反映する。
		m_subscriptions.push_back(eventBus.subscribe<event::ExtensionSwapRequestedEvent>(
		    [this](const event::ExtensionSwapRequestedEvent& e)
		    {
			    swapEquipped(e.m_fromIndex, e.m_toIndex);
		    }));
	}

	void ExtensionEquipSystem::update([[maybe_unused]] float deltaTime)
	{
		if (m_pending.empty())
			return;

		auto* inventory{ m_componentManager.tryGet<component::combat::ExtensionInventoryComponent>(m_playerId) };
		if (inventory == nullptr)
		{
			m_pending.clear();
			return;
		}

		for (const auto type : m_pending)
		{
			// 拾ったものは捨てずに全部持たせる。捨ててしまうと
			// 「何を捨てて何を挿すか」という選択がそもそも発生しない
			const bool willEquip{ inventory->equippedCount() <
				                  component::combat::ExtensionInventoryComponent::MAX_EQUIPPED };
			inventory->m_acquired.push_back(type);

			// 挿せる枠が空いていたぶんだけ、その場で効果を乗せる。
			// 埋まっている場合はリネームブロックで入れ替えるまで効果は乗らない
			if (willEquip)
				applyBonus(type);
		}
		m_pending.clear();
	}

	void ExtensionEquipSystem::swapEquipped(int fromIndex, int toIndex)
	{
		auto* inventory{ m_componentManager.tryGet<component::combat::ExtensionInventoryComponent>(m_playerId) };
		if (inventory == nullptr)
			return;

		const int count{ static_cast<int>(inventory->m_acquired.size()) };
		if (fromIndex < 0 || fromIndex >= count || toIndex < 0 || toIndex >= count)
			return;
		if (fromIndex == toIndex)
			return;

		const auto fromType{ inventory->m_acquired[fromIndex] };
		const auto toType{ inventory->m_acquired[toIndex] };

		std::swap(inventory->m_acquired[fromIndex], inventory->m_acquired[toIndex]);

		// 同じ区分どうしなら並び替えただけで、能力は動かない。
		// 拾ったものを見やすく並べたいという操作は通してよい
		if (inventory->isEquipped(fromIndex) == inventory->isEquipped(toIndex))
		{
			core::log::info("拡張子を並び替え: [{}] <-> [{}]", fromIndex, toIndex);
			return;
		}

		// 装備中の側から見て、抜けたものと入ったものを決める
		const bool isFromEquipped{ inventory->isEquipped(fromIndex) };
		const auto removedType{ isFromEquipped ? fromType : toType };
		const auto addedType{ isFromEquipped ? toType : fromType };

		removeBonus(removedType);
		applyBonus(addedType);

		m_eventBus.publish(event::ExtensionSwappedEvent{ addedType, removedType });

		core::log::info("拡張子を入れ替え: [{}] <-> [{}]", fromIndex, toIndex);
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

	void ExtensionEquipSystem::removeBonus(core::data::FileExtensionType type)
	{
		const auto& bonus{ m_resourceManager.getExtensionBonus(type) };

		if (auto* attack{ m_componentManager.tryGet<component::combat::AttackComponent>(m_playerId) })
		{
			attack->m_attackPower -= bonus.atk;
			attack->m_attackRange -= bonus.attackRange;
			// 加算時に1.0で頭打ちにしているぶん、引くと0を下回りうる
			attack->m_criticalRate = std::max(attack->m_criticalRate - bonus.criticalRate, 0.0f);
		}

		if (auto* health{ m_componentManager.tryGet<component::combat::HealthComponent>(m_playerId) })
		{
			// 現在HPも同じだけ減らす。上限だけ下げて現在値を据え置くと、
			// HP強化を挿しては外すだけで全快でき、入れ替えが回復手段になってしまう
			constexpr float MIN_HP_AFTER_REMOVE{ 1.0f };

			health->m_maxHp -= bonus.hp;
			health->m_defence -= bonus.def;
			health->m_currentHp = std::clamp(health->m_currentHp - bonus.hp,
			    MIN_HP_AFTER_REMOVE, health->m_maxHp);
		}

		if (auto* stats{ m_componentManager.tryGet<component::combat::PlayerStatsComponent>(m_playerId) })
		{
			stats->m_moveSpeed -= bonus.spd;
			stats->m_projectileSpeed -= bonus.projectileSpeed;
			stats->m_projectileRange -= bonus.projectileRange;
		}
	}
} // namespace game::system::combat
