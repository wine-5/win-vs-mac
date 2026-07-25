#include "AnimationSetup.h"
#include "game/constant/AnimationState.h"

namespace
{
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
	component::visual::AnimationComponent buildAnimationComponent(
	    const std::vector<core::data::AnimationClipDef>& defs,
	    core::iface::IResourceManager& resourceManager)
	{
		component::visual::AnimationComponent anim{};
		for (const auto& def : defs)
		{
			component::visual::AnimationClip clip{};
			clip.m_handle = resourceManager.loadAnimationById(def.animId);
			clip.m_isLoop = def.loop;
			clip.m_onComplete = toAnimationState(def.onComplete);
			clip.m_priority = toAnimationPriority(def.priority);
			clip.m_speed = def.speed;
			clip.m_startTime = def.startTime;
			anim.m_clips[toAnimationState(def.state)] = clip;
		}
		return anim;
	}
} // namespace game::actor
