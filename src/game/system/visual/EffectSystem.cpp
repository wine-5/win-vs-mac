#include "game/system/visual/EffectSystem.h"
#include "core/base/ServiceLocator.h"
#include "game/component/visual/EffectComponent.h"
#include "game/component/movement/TransformComponent.h"

namespace game::system::visual
{
	EffectSystem::EffectSystem(core::ecs::ComponentManager& componentManager,
	    core::base::EventBus& eventBus,
	    core::iface::IEffectFactory& effectFactory)
	    : m_componentManager{ componentManager }
	    , m_eventBus{ eventBus }
	    , m_effectFactory{ effectFactory }
	{
		setupEventSubscriptions();
	}

	void EffectSystem::setupEventSubscriptions()
	{
		// AttackHitEventを購読する
		m_subscriptions.push_back(m_eventBus.subscribe<game::event::AttackHitEvent>(
		    [this](const game::event::AttackHitEvent& e)
		    {
				onAttackHit(e);
		    }));

		// AttackStartEventを購読する
		m_subscriptions.push_back(m_eventBus.subscribe<game::event::AttackStartEvent>(
		    [this](const game::event::AttackStartEvent& e)
		    {
			    onAttackStart(e);
		    }));

		// EnemyDeadEventを購読する
		m_subscriptions.push_back(m_eventBus.subscribe<game::event::EnemyDeadEvent>(
		    [this](const game::event::EnemyDeadEvent& e)
		    {
			    onEnemyDead(e);
		    }));

		// EnemySpawnedEventを購読する
		m_subscriptions.push_back(m_eventBus.subscribe<game::event::EnemySpawnedEvent>(
		    [this](const game::event::EnemySpawnedEvent& e)
		    {
			    onEnemySpawned(e);
		    }));
	}


	void EffectSystem::update(float deltaTime)
	{
		// エフェクトの自動終了チェックの処理を委譲する
		m_effectFactory.update();

		auto entities{ m_componentManager.getAllEntities<component::visual::EffectComponent>() };
		for (auto entityId : entities)
		{
			auto& effect{ m_componentManager.get<component::visual::EffectComponent>(entityId) };

			std::erase_if(effect.m_slots, [this](const component::visual::EffectComponent::Slot& slot)
			    { return !m_effectFactory.isPlaying(slot.m_handle); });
		}
	}

	void EffectSystem::playAndTrack(core::ecs::EntityId entityId,
	    core::constant::EffectType type, const core::Vector3& position,
	    const core::Vector3& rotation)
	{
		const int handle{ m_effectFactory.play(type, position, rotation) };
		if (handle == -1)
			return;

		// エフェクトの寿命を追跡するスロットを持たないEntityは記録先が無いので何もしない
		auto* effect{ m_componentManager.tryGet<component::visual::EffectComponent>(entityId) };
		if (effect == nullptr)
			return;

		effect->m_slots.push_back({ type, handle });
	}

	void EffectSystem::onAttackHit(const game::event::AttackHitEvent& event)
	{
		// 被弾したEntityの位置でヒットエフェクトを再生する
		if (const auto* transform{ m_componentManager.tryGet<component::movement::TransformComponent>(event.m_targetId) })
			playAndTrack(event.m_targetId, event.m_effectType, transform->m_position);
	}

	void EffectSystem::onAttackStart(const game::event::AttackStartEvent& event)
	{
		// 攻撃者自身の位置でエフェクトを再生する（斬撃などの演出用）。
		// 斬撃は「どちらへ振ったか」が分かる必要があるため、攻撃者の向きへ合わせ、
		// さらに攻撃の種類ごとの傾き（縦振り／水平回転など）をイベントから受けて足す
		// 音だけを鳴らす攻撃（絵を持たない振り）ではエフェクト種別がNoneで届く
		if (event.m_effectType == core::constant::EffectType::None)
			return;

		const auto* transform{ m_componentManager.tryGet<component::movement::TransformComponent>(event.m_attackerId) };
		if (transform == nullptr)
			return;

		const core::Vector3 rotation{
			transform->m_rotation.x + event.m_effectRotationOffset.x,
			transform->m_rotation.y + event.m_effectRotationOffset.y,
			transform->m_rotation.z + event.m_effectRotationOffset.z
		};

		// 基準はEntityの原点（足元）。斬撃エフェクトは原点から上方向へ伸びる絵柄で、
		// 体を通り抜けるように描かれる前提で作られているため、手の高さを基準にすると
		// そのぶん丸ごと持ち上がって頭より高く出てしまう。高さの微調整はオフセットで行う
		core::Vector3 position{ transform->m_position };
		position.x += event.m_effectPositionOffset.x;
		position.y += event.m_effectPositionOffset.y;
		position.z += event.m_effectPositionOffset.z;

		playAndTrack(event.m_attackerId, event.m_effectType, position, rotation);
	}

	void EffectSystem::onEnemyDead(const game::event::EnemyDeadEvent& event)
	{
		// 死亡した敵の位置で撃破エフェクトを再生する（Tキーのデバッグテストと同じEnemy_HitWindowを使用）
		if (const auto* transform{ m_componentManager.tryGet<component::movement::TransformComponent>(event.m_entityId) })
			playAndTrack(event.m_entityId, core::constant::EffectType::Enemy_HitWindow, transform->m_position);
	}

	void EffectSystem::onEnemySpawned(const game::event::EnemySpawnedEvent& event)
	{
		// スポーン位置はイベントが直接持っているのでTransformComponentは参照しない
		// （複製ハンドルの生成が終わるより前に呼ばれるため、確実に取得できる位置引数を使う）
		playAndTrack(event.m_entityId, core::constant::EffectType::Enemy_Spawn, event.m_position);
	}
} // namespace game::system::visual