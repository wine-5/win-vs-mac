#include "CriticalHandler.h"
#include "game/component/combat/AttackComponent.h"

namespace game::attack
{
	CriticalHandler::CriticalHandler(core::ecs::ComponentManager& componentManager)
	    : m_componentManager{ componentManager }
	{
	}

	void CriticalHandler::setNext(std::unique_ptr<IDamageHandler> next)
	{
		// 次のチェーンに所有権を移動
		m_next = std::move(next);
	}

	void CriticalHandler::handle(DamageChain& chain)
	{
		// 弾のように AttackComponent を持たない攻撃者もあるため、無ければ素通しする
		auto* attack{ m_componentManager.tryGet<component::combat::AttackComponent>(chain.m_attackId) };
		if (attack && attack->m_criticalRate > 0.0f &&
		    m_distribution(m_rng) < attack->m_criticalRate)
		{
			chain.m_damage *= attack->m_criticalMultiplier;
			chain.m_isCritical = true;
		}

		if (m_next)
			m_next->handle(chain);
	}
} // namespace game::attack
