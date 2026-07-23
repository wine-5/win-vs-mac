#include "EnemyDeathSystem.h"
#include "game/component/EnemyTypeComponent.h"
#include "game/component/DeathComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/AnimationComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/TransformComponent.h"
#include "game/constant/EnemyType.h"
#include "game/constant/AnimationState.h"
#include <algorithm>
#include <cmath>

namespace
{
	// 消失フェード（ディゾルブ）にかける時間（秒）
	constexpr float FADE_DURATION{ 0.8f };
	// 赤くなりきるまでの時間（秒）。死亡直後から素早く赤熱させる
	constexpr float RED_BUILD_DURATION{ 0.35f };
	// 死亡アニメが完了イベントを発行しない異常時の保険タイムアウト（秒）。
	// 通常はAnimationFinishedEventでフェード開始するので、これは事故防止用の上限
	constexpr float ANIM_FALLBACK_TIMEOUT{ 5.0f };
	// 落下する敵が着地しない場合の保険タイムアウト（秒）。これを超えたら強制的にフェード開始
	constexpr float FALL_TIMEOUT{ 3.0f };
	// 落下する敵に死亡時に与える下向き初速。高所の浮遊敵を素早く落とすため終端速度で落とす。
	// PhysicsSystemの落下上限(-200)と同値にし、上限クランプを活かして地面すり抜けを防ぐ
	constexpr float FLYING_PLUNGE_SPEED{ -200.0f };

	// 落下死（Safari）の「故障してガタガタ落ちる」演出。着地するまで左右に激しく振れる
	constexpr float DEATH_SHAKE_FREQ{ 9.0f };     // 揺れの速さ（rad/秒）
	constexpr float DEATH_SHAKE_ROLL{ 0.5f };     // 左右ロール（z回転）の振幅[rad]（約29度）
	constexpr float DEATH_SHAKE_PITCH{ 0.07f };   // 前後ピッチ（x回転）の振幅[rad]。別周波数で不規則さを足す
	constexpr float DEATH_SHAKE_LATERAL{ 45.0f }; // 横方向の小刻みなガタつき（水平速度）[単位/秒]

	/**
	 * @brief 落下して着地する死に方をする敵か（浮遊敵）を判定する
	 *
	 * 浮遊する Safari は死亡すると落下・バウンドしてから消える。
	 * 地上敵（Xcode/Mac）は死亡アニメを見せてから消える
	 * @param componentManager ComponentManagerの参照
	 * @param entityId 判定対象のEntityId
	 * @return 落下する敵ならtrue
	 */
	bool isFallingDeath(core::ecs::ComponentManager& componentManager, core::ecs::EntityId entityId)
	{
		using game::component::EnemyTypeComponent;
		const auto* type{ componentManager.tryGet<EnemyTypeComponent>(entityId) };
		return type != nullptr && type->m_type == game::constant::EnemyType::Safari;
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
		m_subscriptions.push_back(m_eventBus.subscribe<event::EnemyDeadEvent>(
		    [this](const event::EnemyDeadEvent& e)
		    { onEnemyDead(e); }));

		// 死亡アニメの再生完了を待ってフェードを始めるため、完了イベントを購読する
		m_subscriptions.push_back(m_eventBus.subscribe<event::AnimationFinishedEvent>(
		    [this](const event::AnimationFinishedEvent& e)
		    { onAnimationFinished(e); }));
	}

	void EnemyDeathSystem::onAnimationFinished(const event::AnimationFinishedEvent& e)
	{
		// 死亡待ち中の敵の「死亡アニメ」が完了したら、消失フェードを解禁する
		if (e.m_state == constant::AnimationState::Dying &&
		    m_componentManager.has<component::DeathComponent>(e.m_entityId))
			m_componentManager.get<component::DeathComponent>(e.m_entityId).m_animFinished = true;
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
		// このアニメの完了（AnimationFinishedEvent）を待ってからフェードに入る。
		// 死亡クリップを持たない敵（Safari等）は待つアニメが無いので、その場でアニメ完了扱いにする
		bool hasDyingClip{ false };
		if (m_componentManager.has<component::AnimationComponent>(e.m_entityId))
		{
			auto& anim{ m_componentManager.get<component::AnimationComponent>(e.m_entityId) };
			if (anim.m_clips.contains(constant::AnimationState::Dying))
			{
				anim.m_requested = constant::AnimationState::Dying;
				hasDyingClip = true;
			}
		}
		if (!hasDyingClip)
			m_componentManager.get<component::DeathComponent>(e.m_entityId).m_animFinished = true;
	}

	void EnemyDeathSystem::update(float deltaTime)
	{
		auto entities{ m_componentManager.getAllEntities<component::DeathComponent>() };
		for (auto entityId : entities)
		{
			auto& death{ m_componentManager.get<component::DeathComponent>(entityId) };
			death.m_elapsed += deltaTime;

			// 落下する敵（Safari）は着地するまで左右にガタガタ揺れながら故障したように落ちる。
			// 機体正面がローカルX軸のため、z回転が左右のロール（傾き）に見える。
			if (isFallingDeath(m_componentManager, entityId) &&
			    m_componentManager.has<component::TransformComponent>(entityId))
			{
				auto& transform{ m_componentManager.get<component::TransformComponent>(entityId) };
				// 地面に触れるまでの落下中だけ揺らす。初回接地でピタッと止める（バウンド完了は待たない）
				if (!death.m_hasTouchedGround)
				{
					const float t{ death.m_elapsed };
					// z回転で左右ロール、x回転を別周波数で混ぜて規則的でない「故障」感を出す
					transform.m_rotation.z = std::sinf(t * DEATH_SHAKE_FREQ) * DEATH_SHAKE_ROLL;
					transform.m_rotation.x = std::sinf(t * DEATH_SHAKE_FREQ * 1.53f) * DEATH_SHAKE_PITCH;
					// 横方向の小刻みなガタつき（平均0で位置は概ね保ちつつ震わせる）
					if (m_componentManager.has<component::VelocityComponent>(entityId))
					{
						auto& velocity{ m_componentManager.get<component::VelocityComponent>(entityId) };
						velocity.m_velocity.x = std::sinf(t * DEATH_SHAKE_FREQ * 0.9f) * DEATH_SHAKE_LATERAL;
						velocity.m_velocity.z = std::cosf(t * DEATH_SHAKE_FREQ * 1.1f) * DEATH_SHAKE_LATERAL;
					}
				}
				else if (m_componentManager.has<component::VelocityComponent>(entityId))
				{
					// 接地後は横のガタつきを止めて死体が滑らないようにする（傾きはそのまま倒れた姿勢で残す）
					auto& velocity{ m_componentManager.get<component::VelocityComponent>(entityId) };
					velocity.m_velocity.x = 0.0f;
					velocity.m_velocity.z = 0.0f;
				}
			}

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
					// 地上敵は死亡アニメが再生完了してから消え始める（保険で上限も設ける）
					readyToFade = death.m_animFinished || death.m_elapsed >= ANIM_FALLBACK_TIMEOUT;

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
				const auto type{ m_componentManager.get<component::EnemyTypeComponent>(entityId).m_type };

				// プールへ返却する前に見た目を元に戻す（次にこのハンドルを使う敵が赤いまま出現しないように）
				m_renderer.resetModelAppearance(modelHandle);
				m_enemySpawner.returnEnemy(type, entityId, modelHandle);
				m_componentManager.removeAll(entityId);
				m_entityManager.destroy(core::ecs::Entity(entityId));
			}
		}
	}
} // namespace game::system
