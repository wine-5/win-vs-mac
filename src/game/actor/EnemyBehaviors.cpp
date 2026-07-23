#include "game/actor/EnemyBehaviors.h"
#include "game/data/EnemyData.h"
#include "game/component/AttackComponent.h"
#include "game/component/AnimationComponent.h"
#include "game/component/AnimationClip.h"
#include "game/component/ai/MeleeChaseAIComponent.h"
#include "game/component/ai/RangeKeepAIComponent.h"
#include "game/component/ai/PatrolComponent.h"
#include "game/component/ai/MacAIComponent.h"
#include "game/component/TelegraphComponent.h"
#include "game/constant/AnimationState.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/ILogger.h"
#include <numbers>
#include <random>
#include <string>
#include <string_view>

namespace
{
	constexpr float DEG_TO_RAD{ std::numbers::pi_v<float> / 180.0f };

	/**
	 * @brief ストレイフ（周回）の向きを個体ごとにランダムに決める
	 * @return 右回り(+1) または 左回り(-1)
	 */
	int pickStrafeDirection()
	{
		static std::mt19937 rng{ std::random_device{}() };
		static std::uniform_int_distribution<int> dist{ 0, 1 };
		return dist(rng) == 0 ? 1 : -1;
	}

	/**
	 * @brief アニメ状態名の文字列をAnimationStateへ変換する（未知はIdle扱い）
	 */
	game::constant::AnimationState toAnimationState(const std::string& name)
	{
		using game::constant::AnimationState;
		if (name == "Walk")
			return AnimationState::Walk;
		if (name == "Run")
			return AnimationState::Run;
		if (name == "Attack1")
			return AnimationState::Attack1;
		if (name == "Attack2")
			return AnimationState::Attack2;
		if (name == "Hit")
			return AnimationState::Hit;
		if (name == "Dying")
			return AnimationState::Dying;
		if (name == "Jump")
			return AnimationState::Jump;
		return AnimationState::Idle;
	}

	/**
	 * @brief 優先度名の文字列を割り込み優先度の数値へ変換する（未知はlocomotion扱い）
	 */
	int toAnimationPriority(const std::string& name)
	{
		namespace priority = game::constant::animation_priority;
		if (name == "dying")
			return priority::DYING;
		if (name == "hit")
			return priority::HIT;
		if (name == "attack")
			return priority::ATTACK;
		if (name == "jump")
			return priority::JUMP;
		return priority::LOCOMOTION;
	}
} // namespace

namespace game::actor
{
	namespace
	{
		void installMeleeChase(core::ecs::ComponentManager& cm, core::ecs::EntityId id, const data::EnemyData&)
		{
			// MeleeChaseAISystemがこのコンポーネントの有無で近接敵を判定する
			cm.add<component::ai::MeleeChaseAIComponent>(id, {});
		}

		void installRangeKeep(core::ecs::ComponentManager& cm, core::ecs::EntityId id, const data::EnemyData& enemyData)
		{
			// RangeKeepAISystem/EnemyRangedAttackSystemがこのコンポーネントで距離維持型を判定する。
			// 距離・浮遊高度・発射間隔・正面軸補正はenemyData（JSONのgameplay）から読む
			component::ai::RangeKeepAIComponent rangeKeep{};
			rangeKeep.m_preferredDistanceMin = enemyData.getPreferredDistanceMin();
			rangeKeep.m_preferredDistanceMax = enemyData.getPreferredDistanceMax();
			rangeKeep.m_hoverHeight = enemyData.getHoverHeight();
			rangeKeep.m_fireCooldown = enemyData.getFireCooldown();
			// JSONは度で持つのでラジアンへ変換して保持する
			rangeKeep.m_facingYawOffset = enemyData.getFacingYawOffset() * DEG_TO_RAD;
			// 周回の向きを個体ごとにランダムに固定する（複数体が散開して包囲する隊形になる）
			rangeKeep.m_strafeDirection = pickStrafeDirection();
			cm.add<component::ai::RangeKeepAIComponent>(id, rangeKeep);
		}

		void installPatrol(core::ecs::ComponentManager& cm, core::ecs::EntityId id, const data::EnemyData&)
		{
			// 索敵範囲外の徘徊状態を共通のPatrolComponentで持つ
			cm.add<component::ai::PatrolComponent>(id, {});
		}

		void installBoss(core::ecs::ComponentManager& cm, core::ecs::EntityId id, const data::EnemyData& enemyData)
		{
			// 近接攻撃の発生タイミングはMacAISystemのFSMが管理するため、
			// AttackComponent側のクールダウンによる二重ゲートを無効化する
			if (cm.has<component::AttackComponent>(id))
				cm.get<component::AttackComponent>(id).m_attackCooldown = 0.0f;

			component::ai::MacAIComponent mac{};
			if (enemyData.getMac().has_value())
				mac.m_config = enemyData.getMac().value();
			else
				LOG_E("boss振る舞いが指定されましたが、mac定義（macData.jsonのmac要素）が見つかりません");
			cm.add<component::ai::MacAIComponent>(id, mac);

			// 攻撃の予兆（溜め中に地面へ危険範囲を表示）用。MacAISystemが溜め中に書き込む
			cm.add<component::TelegraphComponent>(id, {});
		}
	} // namespace

	void installEnemyBehaviors(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId entityId, const data::EnemyData& enemyData)
	{
		for (const auto& name : enemyData.getBehaviors())
		{
			if (name == "meleeChase")
				installMeleeChase(componentManager, entityId, enemyData);
			else if (name == "rangeKeep")
				installRangeKeep(componentManager, entityId, enemyData);
			else if (name == "patrol")
				installPatrol(componentManager, entityId, enemyData);
			else if (name == "boss")
				installBoss(componentManager, entityId, enemyData);
			else
				LOG_E("未知のbehavior名です: {}", name.c_str());
		}
	}

	void installEnemyAnimations(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId entityId, const data::EnemyData& enemyData,
	    core::iface::IResourceManager& resourceManager)
	{
		const auto& defs{ enemyData.getAnimations() };
		if (defs.empty())
			return; // アニメーションを持たない敵（Safari等）は何もしない

		component::AnimationComponent anim{};
		for (const auto& def : defs)
		{
			component::AnimationClip clip{};
			clip.m_handle = resourceManager.loadAnimationById(def.animId);
			clip.m_isLoop = def.loop;
			clip.m_onComplete = toAnimationState(def.onComplete);
			clip.m_priority = toAnimationPriority(def.priority);
			clip.m_speed = def.speed;
			anim.m_clips[toAnimationState(def.state)] = clip;
		}
		componentManager.add<component::AnimationComponent>(entityId, anim);
	}
} // namespace game::actor
