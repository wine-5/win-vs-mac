#include "AnimationSystem.h"
#include "game/component/RenderComponent.h"
#include "game/event/InGameEvents.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "core/utility/Log.h"

namespace
{
	constexpr float ANIMATION_FPS = 30.0f;
}

namespace game::system
{
	AnimationSystem::AnimationSystem(core::ecs::ComponentManager& componentManager,
		core::iface::IAnimator& animator,
		core::base::EventBus& eventBus)
		: m_componentManager{ componentManager }
		, m_animator{ animator }
		, m_eventBus{ eventBus }
	{
	}

	void AnimationSystem::update(float deltaTime)
	{
		auto entities{ m_componentManager.getAllEntities<component::AnimationComponent>() };
		for (auto entityId : entities)
		{
			auto& anim = m_componentManager.get<component::AnimationComponent>(entityId);
			if (anim.m_clips.empty())
				continue;

			auto& render = m_componentManager.get<component::RenderComponent>(entityId);

			// 初回はcurrent状態のクリップをアタッチする
			if (anim.m_animIndex == -1)
				changeAnimation(entityId, anim, render.m_modelHandle, anim.m_current);

			// 切り替え要求の処理
			if (anim.m_requested != anim.m_current)
				tryChangeState(entityId, anim, render.m_modelHandle);

			auto clipIt{ anim.m_clips.find(anim.m_current) };
			if (clipIt == anim.m_clips.end())
				continue;
			const auto& clip{ clipIt->second };

			// 時間更新（ループは巻き戻し、非ループは終端で停止して完了処理）
			anim.m_animTime += deltaTime * ANIMATION_FPS * clip.m_speed;
			if (clip.m_isLoop)
			{
				if (anim.m_animTotalTime > 0.0f)
				{
					while (anim.m_animTime >= anim.m_animTotalTime)
						anim.m_animTime -= anim.m_animTotalTime;
				}
			}
			else if (anim.m_animTime >= anim.m_animTotalTime)
			{
				anim.m_animTime = anim.m_animTotalTime;

				if (!anim.m_isCompleted)
				{
					anim.m_isCompleted = true;
					m_eventBus.publish(event::AnimationFinishedEvent{ entityId, anim.m_current });

					// 完了後は onComplete へ自動遷移（Dying は自分自身を指定して停止し続ける）
					if (clip.m_onComplete != anim.m_current)
						changeAnimation(entityId, anim, render.m_modelHandle, clip.m_onComplete);
				}
			}

			m_animator.updateAnimTime(render.m_modelHandle, anim.m_animIndex, anim.m_animTime);
		}
	}

	void AnimationSystem::tryChangeState(core::ecs::EntityId entityId,
		component::AnimationComponent& anim,
		int modelHandle)
	{
		auto requestedIt{ anim.m_clips.find(anim.m_requested) };
		if (requestedIt == anim.m_clips.end())
		{
			core::log::error("[Anim] entity={} 未登録の状態 {} が要求されました",
			    entityId, constant::toString(anim.m_requested));
			anim.m_requested = anim.m_current;
			return;
		}

		// 再生中クリップの優先度（非ループが完了済みなら優先度を失う）
		int currentPriority{ 0 };
		auto currentIt{ anim.m_clips.find(anim.m_current) };
		if (currentIt != anim.m_clips.end())
		{
			const bool isActive{ currentIt->second.m_isLoop || !anim.m_isCompleted };
			if (isActive)
				currentPriority = currentIt->second.m_priority;
		}

		if (requestedIt->second.m_priority >= currentPriority)
		{
			changeAnimation(entityId, anim, modelHandle, anim.m_requested);
			return;
		}

		// 拒否（要求は保持し、再生完了後に再評価される）
	}

	void AnimationSystem::changeAnimation(core::ecs::EntityId entityId,
		component::AnimationComponent& anim,
		int modelHandle,
		constant::AnimationState newState)
	{
		auto it{ anim.m_clips.find(newState) };
		if (it == anim.m_clips.end() || it->second.m_handle == -1)
		{
			core::log::error("[Anim] entity={} {} のクリップが未登録またはハンドル無効です",
			    entityId, constant::toString(newState));
			return;
		}

		m_animator.changeAnimation(modelHandle, anim.m_animIndex, it->second.m_handle, anim.m_animTotalTime);
		anim.m_animTime = 0.0f;
		anim.m_current = newState;
		anim.m_requested = newState;
		anim.m_isCompleted = false;
	}
} // namespace game::system
