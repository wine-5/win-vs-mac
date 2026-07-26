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
		m_subscriptions.push_back(m_eventBus.subscribe<AttackStartEvent>(
		    [this](const AttackStartEvent& e)
		    { onAttackStart(e); }));

		m_subscriptions.push_back(m_eventBus.subscribe<AttackImpactEvent>(
		    [this](const AttackImpactEvent& e)
		    { onAttackImpact(e); }));

		m_subscriptions.push_back(m_eventBus.subscribe<AttackHitEvent>(
		    [this](const AttackHitEvent& e)
		    { onAttackHit(e); }));

		m_subscriptions.push_back(m_eventBus.subscribe<EnemyDeadEvent>(
		    [this](const EnemyDeadEvent& e)
		    { onEnemyDead(e); }));

		m_subscriptions.push_back(m_eventBus.subscribe<PlayerDeadEvent>(
		    [this](const PlayerDeadEvent& e)
		    { onPlayerDead(e); }));

		m_subscriptions.push_back(m_eventBus.subscribe<EnemyAlertedEvent>(
		    [this](const EnemyAlertedEvent& e)
		    { onEnemyAlerted(e); }));
	}

	void AudioEventListener::onAttackStart(const AttackStartEvent& e)
	{
		// 振り始めの音。当たったかどうかに関係なく、振った事実そのものを返す
		if (e.m_seType == core::constant::SeType::None)
			return;

		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio)
			audio->playSe(e.m_seType);
	}

	void AudioEventListener::onAttackImpact(const AttackImpactEvent& e)
	{
		// 当たる瞬間の音（地面の叩きつけなど）。空振りでも鳴らす
		if (e.m_seType == core::constant::SeType::None)
			return;

		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio)
			audio->playSe(e.m_seType);
	}

	void AudioEventListener::onAttackHit(const AttackHitEvent& e)
	{
		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (!audio) return;

		// 命中音。何が当たったか（近接か・どの弾か）に応じてAttackSystemが種別を決めている
		if (e.m_seType != core::constant::SeType::None)
			audio->playSe(e.m_seType);

		// 会心は命中音に重ねて鳴らす。与えたときだけで、被弾側では鳴らさない
		// （やられた側で派手な音が鳴ると、褒められているのか分からなくなる）
		if (e.m_isCritical && e.m_targetId != m_playerId)
			audio->playSe(core::constant::SeType::Critical);

		// プレイヤーが被弾したときのSE
		if (e.m_targetId == m_playerId)
			audio->playSe(core::constant::SeType::HitPlayer);
	}

	void AudioEventListener::onEnemyDead(const EnemyDeadEvent& /*e*/)
	{
		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio) audio->playSe(core::constant::SeType::DeadEnemy);
	}

	void AudioEventListener::onPlayerDead(const PlayerDeadEvent& /*e*/)
	{
		// HPが尽きた瞬間に鳴らす。死亡演出（アニメ→暗転）の頭に音を置きたいので、
		// 演出完了（PlayerDeathSequenceFinishedEvent）ではなくこちらを使う
		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio)
			audio->playSe(core::constant::SeType::DeadPlayer);
	}

	void AudioEventListener::onEnemyAlerted(const EnemyAlertedEvent& /*e*/)
	{
		// 頭上の通知バッジと同じ瞬間に鳴らす。画面の外にいる敵に気づかれたことも
		// 音なら分かるので、視界に頼らず「見つかった」を伝えられる
		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio)
			audio->playSe(core::constant::SeType::EnemyAlert);
	}
} // namespace game::event
