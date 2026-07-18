#include "EnemyRangedAttackSystem.h"
#include "game/component/ai/RangeKeepAIComponent.h"
#include "game/component/AIComponent.h"
#include "game/component/TransformComponent.h"
#include "game/constant/Tag.h"
#include <cmath>
#include <utility>

namespace
{
	constexpr float AIM_TARGET_HEIGHT{ 80.0f }; // 足元でなく胴体あたりを狙うための高さオフセット
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
		// RangeKeepAIComponentを持つ敵（距離維持型）だけを処理する
		auto entities{ m_componentManager.getAllEntities<component::ai::RangeKeepAIComponent>() };

		for (auto entityId : entities)
		{
			if (!m_componentManager.has<component::AIComponent>(entityId))
				continue;

			auto& ai{ m_componentManager.get<component::AIComponent>(entityId) };
			auto& rangeKeep{ m_componentManager.get<component::ai::RangeKeepAIComponent>(entityId) };

			// 発射クールダウンを進める
			if (rangeKeep.m_currentFireCooldown > 0.0f)
				rangeKeep.m_currentFireCooldown -= deltaTime;

			// 発射間隔が未設定（0以下）の敵は遠距離攻撃しない
			if (rangeKeep.m_fireCooldown <= 0.0f)
				continue;

			// AIが無効・対象未設定なら撃たない
			if (!ai.m_isActive)
				continue;
			if (ai.m_targetEntity.getId() == 0)
				continue;

			auto& transform{ m_componentManager.get<component::TransformComponent>(entityId) };
			auto& targetTransform{ m_componentManager.get<component::TransformComponent>(ai.m_targetEntity.getId()) };

			// ターゲットの胴体あたりを狙う方向ベクトル（3D全軸）
			core::Vector3 direction{
				targetTransform.m_position.x - transform.m_position.x,
				(targetTransform.m_position.y + AIM_TARGET_HEIGHT) - transform.m_position.y,
				targetTransform.m_position.z - transform.m_position.z
			};
			float distance{ std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z) };

			// 索敵範囲外・クールダウン中は撃たない
			if (distance > ai.m_detectionRange)
				continue;
			if (rangeKeep.m_currentFireCooldown > 0.0f)
				continue;

			// 方向ベクトルを正規化
			if (distance > 0.0f)
			{
				direction.x /= distance;
				direction.y /= distance;
				direction.z /= distance;
			}

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
		}
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
