#include "ProjectileSystem.h"
#include "game/component/ProjectileComponent.h"
#include "game/component/AttackComponent.h"
#include "game/event/InGameEvents.h"
#include <algorithm>

namespace game::system::combat
{
	ProjectileSystem::ProjectileSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityManager& entityManager,
	    core::base::EventBus& eventBus)
	    : m_componentManager{ componentManager }, m_entityManager{ entityManager }, m_eventBus{ eventBus }
	{
		// 攻撃がヒットしたとき、攻撃者が弾なら破棄予約する（destroy-on-hit）
		m_subscriptions.push_back(m_eventBus.subscribe<event::AttackHitEvent>(
		    [this](const event::AttackHitEvent& e)
		    {
			    if (m_componentManager.has<component::ProjectileComponent>(e.m_attackerId))
				    m_pendingDestroy.push_back(e.m_attackerId);
		    }));
	}

	void ProjectileSystem::update(float deltaTime)
	{
		auto projectiles{ m_componentManager.getAllEntities<component::ProjectileComponent>() };
		for (auto id : projectiles)
		{
			// 既にヒット等で破棄予約済みの弾は再ヒット判定させない。
			// AttackSystemはProjectileSystemより後に実行されるため、ヒットした弾は
			// このフレームの購読コールバックでm_pendingDestroyに積まれるが、実際の破棄は
			// このupdate関数の末尾まで遅延される。ここでスキップしないと、次フレームの
			// このループでm_attackRequestedが再度trueになり、破棄されるまでの間
			// 毎フレーム再ヒットしてエフェクトが際限なく積み重なってしまう。
			if (std::ranges::find(m_pendingDestroy, id) != m_pendingDestroy.end())
				continue;

			auto& projectile{ m_componentManager.get<component::ProjectileComponent>(id) };
			projectile.m_remainingLifetime -= deltaTime;
			if (projectile.m_remainingLifetime <= 0.0f)
			{
				m_pendingDestroy.push_back(id);
				continue;
			}

			// AttackSystemに毎フレーム拾わせ続ける（接触したらヒット扱いになる）
			if (m_componentManager.has<component::AttackComponent>(id))
				m_componentManager.get<component::AttackComponent>(id).m_attackRequested = true;
		}

		// 破棄予約をまとめて処理する（多重破棄を防ぐため存在チェックしてから消す）
		for (auto id : m_pendingDestroy)
		{
			if (!m_componentManager.has<component::ProjectileComponent>(id))
				continue;
			m_componentManager.removeAll(id);
			m_entityManager.destroy(core::ecs::Entity(id));
		}
		m_pendingDestroy.clear();
	}
} // namespace game::system::combat
