#include "TargetingSystem.h"
#include "game/component/combat/AimComponent.h"
#include "game/component/CameraComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/combat/HealthComponent.h"
#include "game/component/TagComponent.h"
#include <cmath>

namespace
{
	constexpr float EYE_HEIGHT{ 100.0f };       // 照準主体の目線の高さ
	constexpr float TARGET_AIM_HEIGHT{ 75.0f }; // 対象の胴体あたりを狙点にする
	constexpr float ON_TARGET_COS{ 0.99f };     // 視線とのなす角のしきい値（約8度以内で捕捉）
} // namespace

namespace game::system::combat
{
	TargetingSystem::TargetingSystem(core::ecs::ComponentManager& componentManager)
	    : m_componentManager{ componentManager }
	{
	}

	void TargetingSystem::update(float deltaTime)
	{
		// AimComponent＋CameraComponentを持つ主体（カメラで照準するもの）を走査する
		auto aimers{ m_componentManager.getAllEntities<component::combat::AimComponent>() };
		for (auto aimerId : aimers)
		{
			if (!m_componentManager.has<component::CameraComponent>(aimerId) ||
			    !m_componentManager.has<component::movement::TransformComponent>(aimerId) ||
			    !m_componentManager.has<component::TagComponent>(aimerId))
				continue;

			auto& aim{ m_componentManager.get<component::combat::AimComponent>(aimerId) };
			const auto& camera{ m_componentManager.get<component::CameraComponent>(aimerId) };
			const auto& aimerTransform{ m_componentManager.get<component::movement::TransformComponent>(aimerId) };
			const auto aimerTag{ m_componentManager.get<component::TagComponent>(aimerId).m_tag };

			const core::Vector3 origin{
				aimerTransform.m_position.x,
				aimerTransform.m_position.y + EYE_HEIGHT,
				aimerTransform.m_position.z
			};

			// 最も視線に近い「別陣営の被ダメージ対象」を捕捉対象にする
			float bestDot{ ON_TARGET_COS };
			core::ecs::EntityId bestTarget{ core::ecs::INVALID_ENTITY_ID };

			auto candidates{ m_componentManager.getAllEntities<component::combat::HealthComponent>() };
			for (auto targetId : candidates)
			{
				if (targetId == aimerId)
					continue;
				if (!m_componentManager.has<component::movement::TransformComponent>(targetId))
					continue;
				// 同じ陣営は狙わない
				if (m_componentManager.has<component::TagComponent>(targetId) &&
				    m_componentManager.get<component::TagComponent>(targetId).m_tag == aimerTag)
					continue;
				// 死亡した対象は狙わない
				if (m_componentManager.get<component::combat::HealthComponent>(targetId).m_isDead)
					continue;

				const auto& targetTransform{ m_componentManager.get<component::movement::TransformComponent>(targetId) };
				const core::Vector3 toTarget{
					targetTransform.m_position.x - origin.x,
					targetTransform.m_position.y + TARGET_AIM_HEIGHT - origin.y,
					targetTransform.m_position.z - origin.z
				};

				const float length{ std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z) };
				if (length <= 0.0f)
					continue;

				// 視線方向（単位ベクトル）との内積＝cosθ。大きいほど視線に近い
				const float dot{ (toTarget.x * camera.m_forward.x +
					              toTarget.y * camera.m_forward.y +
					              toTarget.z * camera.m_forward.z) /
					             length };
				if (dot > bestDot)
				{
					bestDot = dot;
					bestTarget = targetId;
				}
			}

			aim.m_hasTarget = (bestTarget != core::ecs::INVALID_ENTITY_ID);
			aim.m_targetId = bestTarget;
		}
	}
} // namespace game::system::combat
