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
	// 消失フェード（ディゾルブ）にかける時間（秒）
	constexpr float FADE_DURATION{ 0.8f };
	// 赤くなりきるまでの時間（秒）。死亡直後から素早く赤熱させる
	constexpr float RED_BUILD_DURATION{ 0.35f };
	// 死亡アニメ持ちの敵が「消え始める」までの猶予（秒）。アニメを見せてからフェードに入る
	constexpr float ANIM_HOLD_DURATION{ 1.2f };
	// 落下する敵が着地しない場合の保険タイムアウト（秒）。これを超えたら強制的にフェード開始
	constexpr float FALL_TIMEOUT{ 3.0f };
	// 落下する敵に死亡時に与える下向き初速。高所の浮遊敵を素早く落とすため終端速度で落とす。
	// PhysicsSystemの落下上限(-200)と同値にし、上限クランプを活かして地面すり抜けを防ぐ
	constexpr float FLYING_PLUNGE_SPEED{ -200.0f };

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

	/**
	 * @brief 落下して着地する死に方をする敵か（浮遊敵）を判定する
	 *
	 * 浮遊高度を持つ距離維持型（Safari）は死亡すると落下・バウンドしてから消える。
	 * 地上敵（Xcode/Mac）は死亡アニメを見せてから消える
	 * @param componentManager ComponentManagerの参照
	 * @param entityId 判定対象のEntityId
	 * @return 落下する敵ならtrue
	 */
	bool isFallingDeath(core::ecs::ComponentManager& componentManager, core::ecs::EntityId entityId)
	{
		using game::component::ai::RangeKeepAIComponent;
		return componentManager.has<RangeKeepAIComponent>(entityId) &&
		       componentManager.get<RangeKeepAIComponent>(entityId).m_hoverHeight > 0.0f;
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

		m_componentManager.add<component::DeathComponent>(e.m_entityId, component::DeathComponent{});

		// 死体が横滑り・横飛びしないよう水平速度を止める。垂直は残し、重力で落下させる。
		if (m_componentManager.has<component::VelocityComponent>(e.m_entityId))
		{
			auto& velocity{ m_componentManager.get<component::VelocityComponent>(e.m_entityId) };
			velocity.m_velocity.x = 0.0f;
			velocity.m_velocity.z = 0.0f;

			// 高所の浮遊敵（Safari）は素早く落ちて地面でバウンドするよう下向き初速を与える
			if (isFallingDeath(m_componentManager, e.m_entityId))
				velocity.m_velocity.y = FLYING_PLUNGE_SPEED;
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
			death.m_elapsed += deltaTime;

			const int modelHandle{ m_componentManager.has<component::RenderComponent>(entityId)
				                       ? m_componentManager.get<component::RenderComponent>(entityId).m_modelHandle
				                       : -1 };

			// フェーズ1：まだ消え始めていなければ「消え始めてよいか」を判定する。
			// 落下する敵は着地・静止するまで（＝バウンド演出が終わるまで）不透明を保ち、
			// 地上敵は死亡アニメを見せる猶予を置いてからフェードに入る
			if (!death.m_fading)
			{
				bool readyToFade{ false };
				if (isFallingDeath(m_componentManager, entityId))
					readyToFade = death.m_hasLanded || death.m_elapsed >= FALL_TIMEOUT;
				else
					readyToFade = death.m_elapsed >= ANIM_HOLD_DURATION;

				if (readyToFade)
					death.m_fading = true;
			}

			// フェーズ2：赤化は死亡直後から進め、フェード中のみ不透明度を下げる
			const float redProgress{ std::clamp(death.m_elapsed / RED_BUILD_DURATION, 0.0f, 1.0f) };
			float alpha{ 1.0f };
			if (death.m_fading)
			{
				death.m_fadeTimer += deltaTime;
				alpha = 1.0f - std::clamp(death.m_fadeTimer / FADE_DURATION, 0.0f, 1.0f);
			}
			m_renderer.applyDeathDissolve(modelHandle, redProgress, alpha);

			// フェード完了後にEntity・モデルハンドルを完全に後始末する
			if (death.m_fading && death.m_fadeTimer >= FADE_DURATION)
			{
				const auto type{ inferEnemyType(m_componentManager, entityId) };

				// プールへ返却する前に見た目を元に戻す（次にこのハンドルを使う敵が赤いまま出現しないように）
				m_renderer.resetModelAppearance(modelHandle);
				m_enemySpawner.returnEnemy(type, entityId, modelHandle);
				m_componentManager.removeAll(entityId);
				m_entityManager.destroy(core::ecs::Entity(entityId));
			}
		}
	}
} // namespace game::system
