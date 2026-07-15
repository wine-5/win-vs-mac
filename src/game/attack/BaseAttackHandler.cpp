#include "BaseAttackHandler.h"
#include "game/component/AttackComponent.h"

namespace game::attack
{
	BaseAttackHandler::BaseAttackHandler(core::ecs::ComponentManager& componentManager)
		: m_componentManager{ componentManager }
	{
	}

	void BaseAttackHandler::setNext(std::unique_ptr<IDamageHandler> next)
	{
		// 次のチェーンに所有権を移動
		m_next = std::move(next);
	}

	void BaseAttackHandler::handle(DamageChain& chain)
	{
		auto& attack{ m_componentManager.get<component::AttackComponent>(chain.m_attackId) };
		chain.m_damage = attack.m_attackPower;

		if (m_next)
			m_next->handle(chain);
	}
} // namespace game::attack
