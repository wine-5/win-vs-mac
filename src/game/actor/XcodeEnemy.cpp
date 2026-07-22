#include "XcodeEnemy.h"
#include "game/component/AnimationComponent.h"
#include "game/component/AIComponent.h"
#include "game/component/ai/MeleeChaseAIComponent.h"
#include "game/constant/AnimationId.h"

namespace game::actor
{
	void XcodeEnemy::setupAnimation()
	{
		using constant::AnimationState;
		namespace anim_id = constant::animation_id;
		namespace priority = constant::animation_priority;
		constexpr float WALK_ANIM_SPEED{ 0.6f }; // 歩行アニメの再生速度(見た目の調整)
		// GroundSlamの再生速度はxcodeData.jsonのgameplay.attackAnimSpeedで設定する（溜めattackWindupと揃える）
		const float groundSlamAnimSpeed{ m_enemyData.getAttackAnimSpeed() };

		component::AnimationComponent anim{};
		anim.m_clips[AnimationState::Idle] = { m_resourceManager.loadAnimationById(anim_id::XCODE_IDLE), true };
		anim.m_clips[AnimationState::Walk] = { m_resourceManager.loadAnimationById(anim_id::XCODE_WALK), true, AnimationState::Idle, priority::LOCOMOTION, WALK_ANIM_SPEED };
		anim.m_clips[AnimationState::Attack1] = { m_resourceManager.loadAnimationById(anim_id::XCODE_GROUND_SLAM), false, AnimationState::Idle, priority::ATTACK, groundSlamAnimSpeed };
		anim.m_clips[AnimationState::Dying] = { m_resourceManager.loadAnimationById(anim_id::XCODE_DYING),       false, AnimationState::Dying, priority::DYING };
		m_componentManager.add<component::AnimationComponent>(m_entity.getId(), anim);
	}

	void XcodeEnemy::setupAI()
	{
		// MeleeChaseAISystemがこのコンポーネントの有無で近接敵を判定する
		// 攻撃のワインドアップ（アニメの振り終わりにダメージを与える遅延）は
		// xcodeData.jsonのgameplay.attackWindupで設定する
		m_componentManager.add<component::ai::MeleeChaseAIComponent>(m_entity.getId(), {});
	}
} // namespace game::actor