#include "AnimationSystem.h"
#include "game/component/RenderComponent.h"

namespace
{
	constexpr float ANIMATION_FPS = 30.0f;
}

namespace game::system
{
	AnimationSystem::AnimationSystem(core::ecs::ComponentManager& componentManager,
		core::ecs::EntityId entityId,
		core::iface::IAnimator& animator,
		int idleAnimHandle,
		int walkAnimHandle)
		: m_componentManager{componentManager}
		, m_entityId{entityId}
		, m_animator{animator}
		, m_idleAnimHandle{idleAnimHandle}
		, m_walkAnimHandle{walkAnimHandle}
	{
	}

	void AnimationSystem::update(float deltaTime)
	{
		auto& anim = m_componentManager.get<component::AnimationComponent<constant::PlayerAnimationState>>(m_entityId);
		auto& velocity = m_componentManager.get<component::VelocityComponent>(m_entityId);
		auto& render = m_componentManager.get<component::RenderComponent>(m_entityId);

		// 速度から次の状態を決める
		bool isMoving{(velocity.m_velocity.x != 0.0f || velocity.m_velocity.z != 0.0f)};
		constant::PlayerAnimationState nextState{isMoving
			? constant::PlayerAnimationState::Walk
			: constant::PlayerAnimationState::Idle};

		// 状態が変わったとき、または未アタッチのときにアニメーションを切り替える
		if (anim.m_state != nextState || anim.m_animIndex == -1)
		{
			int nextHandle{(nextState == constant::PlayerAnimationState::Walk)
				? m_walkAnimHandle
				: m_idleAnimHandle};

			changeAnimation(anim, render.m_modelHandle, nextState, nextHandle);
		}

		// アニメーション時間を進め、終端に達したら先頭に巻き戻してループ再生する
		anim.m_animTime += deltaTime * ANIMATION_FPS;
		if (anim.m_animTotalTime > 0.0f)
		{
			while (anim.m_animTime >= anim.m_animTotalTime)
				anim.m_animTime -= anim.m_animTotalTime;
		}

		m_animator.updateAnimTime(render.m_modelHandle, anim.m_animIndex, anim.m_animTime);
	}

	void AnimationSystem::changeAnimation(
		component::AnimationComponent<constant::PlayerAnimationState>& anim,
		int modelHandle,
		constant::PlayerAnimationState newState,
		int newAnimHandle)
	{
		m_animator.changeAnimation(modelHandle, anim.m_animIndex, newAnimHandle, anim.m_animTotalTime);
		anim.m_animTime = 0.0f;
		anim.m_state = newState;
	}
}