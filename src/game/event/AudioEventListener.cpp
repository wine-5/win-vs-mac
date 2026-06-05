#include "AudioEventListener.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IAudioManager.h"
#include "core/constant/SeType.h"

namespace game::event
{
	AudioEventListener::AudioEventListener(core::base::EventBus& eventBus, core::ecs::EntityId playerId)
		: m_eventBus{ eventBus }
		, m_playerId{ playerId }
	{
		m_eventBus.subscribe<AttackHitEvent>(
			[this](const AttackHitEvent& e) { onAttackHit(e); });

		m_eventBus.subscribe<EnemyDeadEvent>(
			[this](const EnemyDeadEvent& e) { onEnemyDead(e); });
	}

	void AudioEventListener::onAttackHit(const AttackHitEvent& e)
	{
		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (!audio) return;

		// 攻撃SEの再生（プレイヤーがジョブに応じた攻撃音）
		if (e.m_seType != core::constant::SeType::None)
		{
			audio->playSe(e.m_seType);
			audio->playSe(core::constant::SeType::HitEnemy);
		}

		// プレイヤーが被弾したときのSE
		if (e.m_targetId == m_playerId)
			audio->playSe(core::constant::SeType::HitPlayer);
	}

	void AudioEventListener::onEnemyDead(const EnemyDeadEvent& /*e*/)
	{
		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio) audio->playSe(core::constant::SeType::DeadEnemy);
	}
}
