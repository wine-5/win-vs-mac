#include "EnemyDeathSystem.h"
#include "game/component/DeathComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/AnimationComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/ai/MeleeChaseAIComponent.h"
#include "game/component/ai/RangeKeepAIComponent.h"
#include "game/component/ai/BossAIComponent.h"
#include "game/constant/EnemyType.h"
#include "game/constant/AnimationState.h"
#include <algorithm>

namespace
{
	// 敵の死亡演出の合計時間（秒）。死亡アニメを見せてから消えるため少し長めに取る。
	// この演出は敵固有なのでSystem側に持つ
	constexpr float DISSOLVE_DURATION{ 1.8f };

	/**
	 * @brief AI専用マーカーコンポーネントの有無からEnemyTypeを判定する
	 * @param componentManager ComponentManagerの参照
	 * @param entityId 判定対象のEntityId
	 * @return 対応するEnemyType
	 */
	game::constant::EnemyType inferEnemyType(core::ecs::ComponentManager& componentManager, core::ecs::EntityId entityId)
	{
		using game::constant::EnemyType;
		if (componentManager.has<game::component::ai::BossAIComponent>(entityId))
			return EnemyType::Mac;
		if (componentManager.has<game::component::ai::RangeKeepAIComponent>(entityId))
			return EnemyType::Safari;
		return EnemyType::Xcode;
	}
} // namespace

namespace game::system
{
	EnemyDeathSystem::EnemyDeathSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityManager& entityManager,
	    core::base::EventBus& eventBus,
	    game::factory::EnemySpawner& enemySpawner,
	    core::iface::IRenderer& renderer)
	    : m_componentManager{ componentManager }
	    , m_entityManager{ entityManager }
	    , m_eventBus{ eventBus }
	    , m_enemySpawner{ enemySpawner }
	    , m_renderer{ renderer }
	{
		m_eventBus.subscribe<event::EnemyDeadEvent>(
		    [this](const event::EnemyDeadEvent& e)
		    { onEnemyDead(e); });
	}

	void EnemyDeathSystem::onEnemyDead(const event::EnemyDeadEvent& e)
	{
		if (m_componentManager.has<component::DeathComponent>(e.m_entityId))
			return;

		m_componentManager.add<component::DeathComponent>(e.m_entityId, component::DeathComponent{ DISSOLVE_DURATION });

		// 死体が横滑り・横飛びしないよう水平速度を止める。垂直は残し、重力で落下させる。
		// Safari（浮遊敵）はこれで浮遊維持が切れたぶん、ほぼ真下へ落ちてから地面でバウンドする
		if (m_componentManager.has<component::VelocityComponent>(e.m_entityId))
		{
			auto& velocity{ m_componentManager.get<component::VelocityComponent>(e.m_entityId) };
			velocity.m_velocity.x = 0.0f;
			velocity.m_velocity.z = 0.0f;
		}

		// 死亡アニメーションを持つ敵（Xcode/Mac）は最優先度で再生を要求する。
		// クリップ未登録の敵（Safari等）はAnimationSystem側で無視されるだけなので分岐不要
		if (m_componentManager.has<component::AnimationComponent>(e.m_entityId))
		{
			auto& anim{ m_componentManager.get<component::AnimationComponent>(e.m_entityId) };
			if (anim.m_clips.contains(constant::AnimationState::Dying))
				anim.m_requested = constant::AnimationState::Dying;
		}
	}

	void EnemyDeathSystem::update(float deltaTime)
	{
		auto entities{ m_componentManager.getAllEntities<component::DeathComponent>() };
		for (auto entityId : entities)
		{
			auto& death{ m_componentManager.get<component::DeathComponent>(entityId) };
			death.m_timer -= deltaTime;

			const int modelHandle{ m_componentManager.has<component::RenderComponent>(entityId)
				                       ? m_componentManager.get<component::RenderComponent>(entityId).m_modelHandle
				                       : -1 };

			// 進行度に応じた赤化＋ディゾルブ（アルファフェード）を毎フレーム適用する
			const float progress{ 1.0f - std::clamp(death.m_timer / DISSOLVE_DURATION, 0.0f, 1.0f) };
			m_renderer.applyDeathDissolve(modelHandle, progress);

			if (death.m_timer > 0.0f)
				continue;

			const auto type{ inferEnemyType(m_componentManager, entityId) };

			// プールへ返却する前に見た目を元に戻しておく（次にこのハンドルを使う敵が赤いまま出現しないように）
			m_renderer.resetModelAppearance(modelHandle);
			m_enemySpawner.returnEnemy(type, entityId, modelHandle);
			m_componentManager.removeAll(entityId);
			m_entityManager.destroy(core::ecs::Entity(entityId));
		}
	}
} // namespace game::system
