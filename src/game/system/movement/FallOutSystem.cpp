#include "FallOutSystem.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/movement/FallRecoveryComponent.h"
#include "game/component/combat/HealthComponent.h"
#include "game/component/visual/HitEffectComponent.h"
#include "game/component/TagComponent.h"
#include "game/constant/Tag.h"
#include "game/event/InGameEvents.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IAudioManager.h"
#include "core/constant/SeType.h"

namespace
{
	// 最後に立っていた場所からこれ以上下がったら、奈落へ落ちたとみなす。
	// 坂や段差による正規の落差より十分大きくとる
	constexpr float FALL_LIMIT{ 1200.0f };
	// 奈落へ落ちたプレイヤーに与えるダメージ
	constexpr float FALL_DAMAGE{ 100.0f };
} // namespace

namespace game::system::movement
{
	FallOutSystem::FallOutSystem(core::ecs::ComponentManager& componentManager, core::base::EventBus& eventBus)
	    : m_componentManager{ componentManager }
	    , m_eventBus{ eventBus }
	{
	}

	bool FallOutSystem::hasFallenOut(core::ecs::EntityId entityId) const
	{
		const auto* recovery{ m_componentManager.tryGet<component::movement::FallRecoveryComponent>(entityId) };
		if (recovery == nullptr || !recovery->m_hasSafePosition)
			return false;

		// 深く潜るステージなので、絶対的な高さではなく「最後の足場からの落差」で見る
		const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(entityId) };
		return recovery->m_lastSafePosition.y - transform.m_position.y >= FALL_LIMIT;
	}

	void FallOutSystem::punishPlayer(core::ecs::EntityId entityId)
	{
		auto& transform{ m_componentManager.get<component::movement::TransformComponent>(entityId) };
		auto& velocity{ m_componentManager.get<component::movement::VelocityComponent>(entityId) };
		const auto& recovery{ m_componentManager.get<component::movement::FallRecoveryComponent>(entityId) };

		// HPが尽きても直前の足場へ戻す。死亡演出は足場の上で見せたいため
		transform.m_position = recovery.m_lastSafePosition;
		velocity.m_velocity = core::Vector3{};
		velocity.m_externalVelocity = core::Vector3{};

		auto* health{ m_componentManager.tryGet<component::combat::HealthComponent>(entityId) };
		if (health == nullptr || health->m_isDead)
			return;

		health->m_currentHp -= FALL_DAMAGE;
		if (health->m_currentHp < 0.0f)
			health->m_currentHp = 0.0f;

		// 被弾と同じ点滅・SEで「ダメージを受けた」ことを伝える。
		// 落下には攻撃者がいないためAttackHitEventは発行せず、ここで直接鳴らす
		if (auto* hitEffect{ m_componentManager.tryGet<component::visual::HitEffectComponent>(entityId) })
		{
			hitEffect->m_isActive = true;
			hitEffect->m_durationTimer = hitEffect->m_duration;
			hitEffect->m_blinkTimer = hitEffect->m_blinkInterval;
		}
		if (auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() })
			audio->playSe(core::constant::SeType::HitPlayer);

		if (health->m_currentHp <= 0.0f)
		{
			health->m_isDead = true;
			m_eventBus.publish(event::PlayerDeadEvent{});
		}
	}

	void FallOutSystem::killEnemy(core::ecs::EntityId entityId)
	{
		auto* health{ m_componentManager.tryGet<component::combat::HealthComponent>(entityId) };
		if (health == nullptr || health->m_isDead)
			return;

		// 奈落の底から復帰する術は無いので、落ちきった時点で撃破扱いにする。
		// 以降の消失演出・Entityの後始末はEnemyDeathSystemが引き受ける
		health->m_currentHp = 0.0f;
		health->m_isDead = true;
		m_eventBus.publish(event::EnemyDeadEvent{ entityId });
	}

	void FallOutSystem::update(float /*deltaTime*/)
	{
		const auto entities{ m_componentManager.getAllEntities<component::movement::FallRecoveryComponent>() };
		for (const auto entityId : entities)
		{
			if (!hasFallenOut(entityId))
				continue;

			const auto* tag{ m_componentManager.tryGet<component::TagComponent>(entityId) };
			if (tag == nullptr)
				continue;

			if (tag->m_tag == constant::Tag::Player)
				punishPlayer(entityId);
			else if (tag->m_tag == constant::Tag::Enemy)
				killEnemy(entityId);
		}
	}
} // namespace game::system::movement
