#include "ChargeZoomSystem.h"
#include "game/component/combat/PlayerChargeComponent.h"
#include "game/component/camera/CameraEffectComponent.h"

namespace
{
	constexpr float MAX_ZOOM_AMOUNT{ 0.1f }; // 最大溜めでFOVを絞る量（1.0 - 0.2 = 0.8倍まで寄る）
	constexpr float ZOOM_LERP_SPEED{ 6.0f }; // 目標FOV倍率への追従速度（大きいほど機敏）
} // namespace

namespace game::system::camera
{
	ChargeZoomSystem::ChargeZoomSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId playerId)
	    : m_componentManager{ componentManager }
	    , m_playerId{ playerId }
	{
	}

	void ChargeZoomSystem::update(float deltaTime)
	{
		if (!m_componentManager.has<component::combat::PlayerChargeComponent>(m_playerId) ||
		    !m_componentManager.has<component::camera::CameraEffectComponent>(m_playerId))
			return;

		const auto& charge{ m_componentManager.get<component::combat::PlayerChargeComponent>(m_playerId) };
		auto& effect{ m_componentManager.get<component::camera::CameraEffectComponent>(m_playerId) };

		// 溜め中は溜め率に応じてFOVを絞る。離していれば通常（1.0）へ戻す。
		const float targetScale{ charge.m_isCharging
			                         ? 1.0f - MAX_ZOOM_AMOUNT * charge.m_chargeRate
			                         : 1.0f };

		// 目標倍率へフレームレート非依存で指数補間する（急なFOV変化を避ける）
		const float t{ ZOOM_LERP_SPEED * deltaTime };
		const float lerpRate{ t < 1.0f ? t : 1.0f };
		effect.m_fovScale += (targetScale - effect.m_fovScale) * lerpRate;
	}
} // namespace game::system::camera
