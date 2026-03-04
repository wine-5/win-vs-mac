#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IAnimator.h"
#include "game/component/AnimationComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/constant/PlayerAnimationState.h"

namespace game::system
{
	/**
	 * @brief アニメーション状態を管理・更新するSystem
	 */
	class AnimationSystem : public core::ecs::ISystem
	{
	public:
		AnimationSystem(core::ecs::ComponentManager& componentManager,
			core::ecs::EntityId playerId,
			core::iface::IAnimator& animator,
			int idleAnimHandle,
			int walkAnimHandle);

		void update(float deltaTime) override;

	private:
		void changeAnimation(
			component::AnimationComponent<constant::PlayerAnimationState>& anim,
			int modelHandle,
			constant::PlayerAnimationState newState,
			int newAnimHandle);
		
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_playerId;
		core::iface::IAnimator& m_animator;
		int m_idleAnimHandle;
		int m_walkAnimHandle;
	};
}