#include "MacEnemy.h"
#include "game/component/AnimationComponent.h"
#include "game/component/AIComponent.h"
#include "game/component/AttackComponent.h"
#include "game/component/ai/BossAIComponent.h"
#include "game/constant/AnimationId.h"
#include "game/constant/ModelId.h"
#include "core/interface/ILogger.h"

namespace game::actor
{
	void MacEnemy::setupAnimation()
	{
		using constant::AnimationState;
		namespace anim_id = constant::animation_id;
		namespace priority = constant::animation_priority;
		constexpr float WALK_ANIM_SPEED{ 0.6f }; // 歩行アニメの再生速度(見た目の調整)
		constexpr float DASH_ANIM_SPEED{ 0.8f }; // ダッシュのアニメの再生速度(見た目の調整)

		component::AnimationComponent anim{};
		anim.m_clips[AnimationState::Idle]    = { m_resourceManager.loadAnimationById(anim_id::MAC_IDLE), true };
		anim.m_clips[AnimationState::Walk] = { m_resourceManager.loadAnimationById(anim_id::MAC_WALK), true, AnimationState::Idle, priority::LOCOMOTION, WALK_ANIM_SPEED };
		anim.m_clips[AnimationState::Run] = { m_resourceManager.loadAnimationById(anim_id::MAC_RUN), true, AnimationState::Idle, priority::LOCOMOTION, DASH_ANIM_SPEED };
		anim.m_clips[AnimationState::Attack1] = { m_resourceManager.loadAnimationById(anim_id::MAC_SWING_ATTACK), false, AnimationState::Idle,  priority::ATTACK };
		anim.m_clips[AnimationState::Attack2] = { m_resourceManager.loadAnimationById(anim_id::MAC_MAGIC_ATTACK), false, AnimationState::Idle,  priority::ATTACK };
		anim.m_clips[AnimationState::Dying]   = { m_resourceManager.loadAnimationById(anim_id::MAC_DYING),       false, AnimationState::Dying, priority::DYING };
		m_componentManager.add<component::AnimationComponent>(m_entity.getId(), anim);
	}

	void MacEnemy::setupAI()
	{
		auto& ai = m_componentManager.get<component::AIComponent>(m_entity.getId());
		ai.m_behavior = constant::AIBehavior::Boss;

		// 近接攻撃の発生タイミングはBossAISystemのFSM（actionInterval）が管理するため、
		// AttackComponent側のクールダウンによる二重ゲートを無効化する
		if (m_componentManager.has<component::AttackComponent>(m_entity.getId()))
			m_componentManager.get<component::AttackComponent>(m_entity.getId()).m_attackCooldown = 0.0f;

		// BossAISystemがこのコンポーネントの有無でボスを判定する。
		// フェーズ定義（macData.jsonのboss要素）を読み込んで載せる
		component::ai::BossAIComponent boss{};
		auto meta{ m_resourceManager.getMetadata(constant::model_id::ENEMY_MAC) };
		if (meta.has_value() && meta->boss.has_value())
			boss.m_config = meta->boss.value();
		else
			LOG_E("Macのボス定義(boss)がmacData.jsonに見つかりません");

		m_componentManager.add<component::ai::BossAIComponent>(m_entity.getId(), boss);
	}

} // namespace game::actor