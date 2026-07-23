#include "EnemyRangedAttackSystem.h"
#include "game/component/ai/RangeKeepAIComponent.h"
#include "game/component/ai/AIComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/constant/Tag.h"
#include <cmath>
#include <utility>

namespace
{
	constexpr float AIM_TARGET_HEIGHT{ 80.0f }; // 足元でなく胴体あたりを狙うための高さオフセット

	// 攻撃直前の予備動作（ためて膨らむ＋ブルブル）と発射時ポップの調整値。
	// この機体は正面がローカルX軸で、Eulerのどの軸に傾きを入れても左右傾きに見えてしまうため、
	// 回転は使わずスケールの膨張・振動だけで「ためて放つ」感を出す
	constexpr float TELEGRAPH_TIME{ 0.6f };   // 発射前にため動作を始める残り時間（秒）
	constexpr float CHARGE_SWELL{ 0.12f };    // ため中にじわっと膨らむ最大倍率（+12%）
	constexpr float SHAKE_FREQ{ 30.0f };      // ブルブル振動（スケール脈動）の速さ
	constexpr float SHAKE_SCALE_AMP{ 0.05f }; // ブルブル振動の最大倍率（±5%）
	constexpr float LUNGE_TIME{ 0.25f };      // 発射直後のポップの継続時間（秒）
	constexpr float SCALE_POP{ 0.30f };       // 発射の瞬間に膨らむ最大倍率（+30%）
} // namespace

namespace game::system::ai
{
	EnemyRangedAttackSystem::EnemyRangedAttackSystem(core::ecs::ComponentManager& componentManager,
	    factory::ProjectileFactory& projectileFactory,
	    core::data::ProjectileMetadata metadata,
	    std::vector<RangedProjectileVisual> visuals)
	    : m_componentManager{ componentManager }
	    , m_projectileFactory{ projectileFactory }
	    , m_metadata{ std::move(metadata) }
	    , m_visuals{ std::move(visuals) }
	{
	}

	void EnemyRangedAttackSystem::update(float deltaTime)
	{
		m_elapsedTime += deltaTime; // 振動の位相計算に使う

		// RangeKeepAIComponentを持つ敵（距離維持型）だけを処理する
		auto entities{ m_componentManager.getAllEntities<component::ai::RangeKeepAIComponent>() };

		for (auto entityId : entities)
		{
			if (!m_componentManager.has<component::ai::AIComponent>(entityId))
				continue;

			auto& ai{ m_componentManager.get<component::ai::AIComponent>(entityId) };
			auto& rangeKeep{ m_componentManager.get<component::ai::RangeKeepAIComponent>(entityId) };

			// 各種タイマーを進める
			if (rangeKeep.m_currentFireCooldown > 0.0f)
				rangeKeep.m_currentFireCooldown -= deltaTime;
			if (rangeKeep.m_attackAnimTimer > 0.0f)
				rangeKeep.m_attackAnimTimer -= deltaTime;

			// 発射間隔が未設定（0以下）の敵は遠距離攻撃しない
			if (rangeKeep.m_fireCooldown <= 0.0f)
				continue;

			auto& transform{ m_componentManager.get<component::movement::TransformComponent>(entityId) };

			// 予備動作のスケール演出用に基準スケールを一度だけ取得する
			if (rangeKeep.m_baseScale.x == 0.0f && rangeKeep.m_baseScale.y == 0.0f && rangeKeep.m_baseScale.z == 0.0f)
				rangeKeep.m_baseScale = transform.m_scale;

			// プレイヤーが索敵範囲内かどうかと、狙う方向を求める
			bool inRange{ false };
			core::Vector3 direction{};
			float distance{ 0.0f };
			if (ai.m_isActive && ai.m_targetEntity.getId() != 0)
			{
				const auto& targetTransform{ m_componentManager.get<component::movement::TransformComponent>(ai.m_targetEntity.getId()) };
				direction = core::Vector3{
					targetTransform.m_position.x,
					targetTransform.m_position.y + AIM_TARGET_HEIGHT,
					targetTransform.m_position.z
				} - transform.m_position;
				distance = direction.length();
				inRange = (distance <= ai.m_detectionRange);
			}

			// 予備動作（ため→前のめり＋ブルブル＋発射ポップ）を毎フレーム反映する
			applyAttackAnimation(transform, rangeKeep, inRange);

			// 索敵範囲内かつクールダウン完了なら発射する
			if (!inRange || rangeKeep.m_currentFireCooldown > 0.0f)
				continue;

			direction = direction.normalized();

			// 発射源より少し前方から出す（発射者自身への即着弾を避ける）
			core::Vector3 origin{
				transform.m_position.x + direction.x * m_metadata.m_spawnForward,
				transform.m_position.y + m_metadata.m_spawnHeight + direction.y * m_metadata.m_spawnForward,
				transform.m_position.z + direction.z * m_metadata.m_spawnForward
			};

			// 見た目（モデル）と当たり判定半径を1組ランダムに選ぶ
			const RangedProjectileVisual visual{ pickRandomVisual() };

			factory::ProjectileConfig config{};
			config.m_speed = m_metadata.m_speed;
			config.m_damage = m_metadata.m_damage;
			config.m_lifetime = m_metadata.m_lifetime;
			config.m_radius = visual.m_radius; // モデル実寸から自動計算 or 手動指定の半径
			config.m_scale = m_metadata.m_scale;
			config.m_modelHandle = visual.m_modelHandle;

			m_projectileFactory.spawn(origin, direction, config, constant::Tag::Enemy);
			rangeKeep.m_currentFireCooldown = rangeKeep.m_fireCooldown;
			rangeKeep.m_attackAnimTimer = LUNGE_TIME; // 発射直後の前のめり＋ポップを開始する
		}
	}

	void EnemyRangedAttackSystem::applyAttackAnimation(component::movement::TransformComponent& transform,
	    component::ai::RangeKeepAIComponent& rangeKeep, bool inRange)
	{
		float scaleFactor{ 1.0f }; // 基準スケールに対する倍率

		if (rangeKeep.m_attackAnimTimer > 0.0f)
		{
			// 発射直後：ポンと膨らむ（時間経過で減衰して元に戻る）
			const float t{ rangeKeep.m_attackAnimTimer / LUNGE_TIME }; // 1→0
			scaleFactor = 1.0f + SCALE_POP * t;
		}
		else if (inRange && rangeKeep.m_currentFireCooldown < TELEGRAPH_TIME)
		{
			// 発射直前：じわっと膨らむ（ため）＋スケール脈動（ブルブル）。発射が近いほど強くなる
			const float charge{ (TELEGRAPH_TIME - rangeKeep.m_currentFireCooldown) / TELEGRAPH_TIME }; // 0→1
			const float pulse{ std::sinf(m_elapsedTime * SHAKE_FREQ) * SHAKE_SCALE_AMP * charge };
			scaleFactor = 1.0f + CHARGE_SWELL * charge + pulse;
		}

		// 回転は使わない（この機体はEulerだとどの軸でも左右傾きに見えるため）。
		// 予備動作・発射演出はスケールの膨張・脈動のみで表現する。
		// rotation はRangeKeepAISystemの向き制御に任せ、ここでは触らない
		transform.m_scale = {
			rangeKeep.m_baseScale.x * scaleFactor,
			rangeKeep.m_baseScale.y * scaleFactor,
			rangeKeep.m_baseScale.z * scaleFactor
		};
	}

	RangedProjectileVisual EnemyRangedAttackSystem::pickRandomVisual()
	{
		if (m_visuals.empty())
			return RangedProjectileVisual{};

		// m_visualsの中からランダムで選出する（範囲をm_visuals.size()内に制限）
		std::uniform_int_distribution<std::size_t> dist{ 0, m_visuals.size() - 1 };
		return m_visuals[dist(m_rng)];
	}
} // namespace game::system::ai
