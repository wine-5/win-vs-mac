#include "DefenseHandler.h"
#include "game/component/HealthComponent.h"

namespace game::attack
{
	DefenseHandler::DefenseHandler(core::ecs::ComponentManager& componentManager)
		: m_componentManager{ componentManager }
	{
	}

	void DefenseHandler::setNext(std::unique_ptr<IDamageHandler> next)
	{
		// 次のチェーンに所有権を移動
		m_next = std::move(next);
	}

	void DefenseHandler::handle(DamageChain& chain)
	{
		auto& health{ m_componentManager.get<component::HealthComponent>(chain.m_targetId) };
		chain.m_damage -= health.m_defence;

		// 攻撃力より防御力のほうが高いときに攻撃を0にする（そうしないと回復する）
		if (chain.m_damage < 0.0f)
			chain.m_damage = 0.0f;

		if (m_next)
			m_next->handle(chain);
	}
} // namespace game::attack
