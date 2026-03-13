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
		/**
		 * @brief AnimationSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param entityId 対象EntityのID
		 * @param animator IAnimatorの参照
		 * @param idleAnimHandle Idleアニメーションハンドル
		 * @param walkAnimHandle Walkアニメーションハンドル
		 */
		AnimationSystem(core::ecs::ComponentManager& componentManager,
			core::ecs::EntityId entityId,
			core::iface::IAnimator& animator,
			int idleAnimHandle,
			int walkAnimHandle);

		/**
		 * @brief アニメーション状態を管理・更新する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	private:
		void changeAnimation(
			component::AnimationComponent<constant::PlayerAnimationState>& anim,
			int modelHandle,
			constant::PlayerAnimationState newState,
			int newAnimHandle);
		
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_entityId{};
		core::iface::IAnimator& m_animator;
		int m_idleAnimHandle{-1};
		int m_walkAnimHandle{-1};
	};
}