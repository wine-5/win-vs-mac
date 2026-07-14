#pragma once
#include <unordered_map>
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/base/EventBus.h"
#include "core/interface/IAnimator.h"
#include "game/component/AnimationComponent.h"
#include "game/constant/AnimationState.h"

namespace game::system
{
	/**
	 * @brief 全エンティティのアニメーション切り替え・再生を一元管理するSystem
	 *
	 * 他のSystemは AnimationComponent::m_requested に状態を書くだけでよい。
	 * 本Systemが優先度判定・attach/detach・ループ/完了処理を行う。
	 * 非ループクリップの再生完了時は AnimationFinishedEvent を発行する。
	 */
	class AnimationSystem : public core::ecs::ISystem
	{
	public:
		/**
		 * @brief AnimationSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param animator IAnimatorの参照
		 * @param eventBus 完了イベント発行用のEventBus
		 */
		AnimationSystem(core::ecs::ComponentManager& componentManager,
			core::iface::IAnimator& animator,
			core::base::EventBus& eventBus);

		/**
		 * @brief 全エンティティのアニメーション状態を更新する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	private:
		/**
		 * @brief 要求された状態への遷移を優先度に基づいて試みる
		 * @param entityId 対象EntityのID
		 * @param anim 対象のAnimationComponent
		 * @param modelHandle 描画モデルハンドル
		 */
		void tryChangeState(core::ecs::EntityId entityId,
			component::AnimationComponent& anim,
			int modelHandle);

		/**
		 * @brief アニメーションを切り替える（優先度判定なしの強制遷移）
		 * @param entityId 対象EntityのID（ログ用）
		 * @param anim 対象のAnimationComponent
		 * @param modelHandle 描画モデルハンドル
		 * @param newState 遷移先の状態
		 */
		void changeAnimation(core::ecs::EntityId entityId,
			component::AnimationComponent& anim,
			int modelHandle,
			constant::AnimationState newState);

		core::ecs::ComponentManager& m_componentManager;
		core::iface::IAnimator& m_animator;
		core::base::EventBus& m_eventBus;
	};
}
