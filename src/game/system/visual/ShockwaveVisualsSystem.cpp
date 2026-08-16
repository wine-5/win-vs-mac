#include "ShockwaveVisualsSystem.h"
#include <algorithm>
#include "core/utility/Color.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/visual/ShockwaveComponent.h"

namespace
{
	constexpr float GROUND_LIFT{ 3.0f };

	// 3D線は1pxしかないため、半径をずらして重ねることで太い輪に見せる
	constexpr int STROKE_COUNT{ 8 };
	constexpr float STROKE_GAP{ 6.0f };

	constexpr int MAX_ALPHA{ 255 };

	// 最後の波をどれだけ弱めるか（0で減衰なし、1で消える）
	constexpr float RING_DECAY{ 0.35f };

	float easeOut(float t)
	{
		t = std::clamp(t, 0.0f, 1.0f);
		return 1.0f - (1.0f - t) * (1.0f - t);
	}
} // namespace

namespace game::system::visual
{
	ShockwaveVisualsSystem::ShockwaveVisualsSystem(core::ecs::ComponentManager& componentManager,
	    core::iface::IRenderer& renderer)
	    : m_componentManager{ componentManager }
	    , m_renderer{ renderer }
	{
	}

	void ShockwaveVisualsSystem::update(float deltaTime)
	{
		for (const auto entityId :
		    m_componentManager.getAllEntities<component::visual::ShockwaveComponent>())
		{
			auto& shockwave{ m_componentManager.get<component::visual::ShockwaveComponent>(entityId) };

			if (!shockwave.m_isPlaying)
				continue;

			shockwave.m_elapsedTime += deltaTime;
			const float lastRingStart{
				shockwave.m_ringInterval * static_cast<float>(shockwave.m_ringCount - 1)
			};
			if (shockwave.m_elapsedTime >= lastRingStart + shockwave.m_duration)
				shockwave.m_isPlaying = false;
		}
	}

	void ShockwaveVisualsSystem::draw()
	{
		for (const auto entityId :
		    m_componentManager.getAllEntities<component::visual::ShockwaveComponent>())
		{
			const auto& shockwave{
				m_componentManager.get<component::visual::ShockwaveComponent>(entityId)
			};
			if (!shockwave.m_isPlaying || shockwave.m_duration <= 0.0f)
				continue;

			core::Vector3 center{ shockwave.m_origin };
			center.y += GROUND_LIFT;

			for (int ring{ 0 }; ring < shockwave.m_ringCount; ++ring)
			{
				const float ringTime{ shockwave.m_elapsedTime - shockwave.m_ringInterval * static_cast<float>(ring) };
				if (ringTime < 0.0f || ringTime > shockwave.m_duration)
					continue;

				const float progress{ ringTime / shockwave.m_duration };
				const float radius{ shockwave.m_startRadius + (shockwave.m_maxRadius - shockwave.m_startRadius) * easeOut(progress) };
				// 後の波ほど弱めるが、消えるほどは落とさない。
				// 本数で割ると3本目が1/3の濃さになり、独立した波として見えなくなる
				const float ringRatio{ shockwave.m_ringCount > 1
					                       ? static_cast<float>(ring) / static_cast<float>(shockwave.m_ringCount - 1)
					                       : 0.0f };
				const float ringFade{ 1.0f - RING_DECAY * ringRatio };
				const int alpha{ static_cast<int>(MAX_ALPHA * (1.0f - progress) * ringFade) };
				if (alpha <= 0)
					continue;

				const unsigned int color{ core::utility::Color::argb(alpha,
					static_cast<int>((shockwave.m_color >> 16) & 0xFF),
					static_cast<int>((shockwave.m_color >> 8) & 0xFF),
					static_cast<int>(shockwave.m_color & 0xFF)) };

				if (shockwave.m_isFilled)
				{
					m_renderer.drawGroundCircle(center, radius, color, true);
					continue;
				}

				for (int stroke{ 0 }; stroke < STROKE_COUNT; ++stroke)
				{
					const float strokeRadius{ radius - STROKE_GAP * static_cast<float>(stroke) };
					if (strokeRadius <= 0.0f)
						break;
					m_renderer.drawGroundCircle(center, strokeRadius, color, false);
				}
			}
		}
	}
} // namespace game::system::visual
