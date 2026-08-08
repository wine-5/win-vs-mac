#include "BlockDebrisSystem.h"
#include "game/component/stage/BlockDebrisComponent.h"
#include "game/component/stage/DestructibleComponent.h"
#include "core/ecs/Entity.h"
#include <algorithm>

namespace
{
	/// @brief 破片にかかる重力（ユニット/秒^2）
	constexpr float GRAVITY{ 1400.0f };

	/// @brief 床で跳ね返るときに残る速度の割合
	constexpr float BOUNCE_RETENTION{ 0.34f };

	/// @brief 床との摩擦で水平速度に掛ける係数
	constexpr float FLOOR_FRICTION{ 0.72f };

	/// @brief 空気抵抗（1秒あたりに失う速度の割合）
	constexpr float AIR_DRAG{ 0.9f };

	/// @brief 跳ねるたびに回転が落ち着く割合
	constexpr float SPIN_DAMPING{ 0.6f };

	/// @brief 寿命のうち、縮み始めるまでの割合
	///
	/// フレーム単位では半透明にできないため、縮小で消す。
	/// 最初から縮み始めると飛んだ瞬間に小さくなって迫力が出ないので、後半だけ縮める
	constexpr float SHRINK_START_RATIO{ 0.5f };
} // namespace

namespace game::system::stage
{
	BlockDebrisSystem::BlockDebrisSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityManager& entityManager,
	    core::iface::IRenderer& renderer)
	    : m_componentManager{ componentManager }
	    , m_entityManager{ entityManager }
	    , m_renderer{ renderer }
	{
	}

	void BlockDebrisSystem::update(float deltaTime)
	{
		const auto entities{ m_componentManager.getAllEntities<component::stage::BlockDebrisComponent>() };
		for (const auto entityId : entities)
		{
			auto& debris{ m_componentManager.get<component::stage::BlockDebrisComponent>(entityId) };
			debris.m_elapsed += deltaTime;

			// 飛散し終わったブロックは以後どこからも参照されないため破棄する
			if (debris.m_elapsed >= debris.m_lifetime)
			{
				m_componentManager.removeAll(entityId);
				m_entityManager.destroy(core::ecs::Entity(entityId));
				continue;
			}

			const float shrinkStart{ debris.m_lifetime * SHRINK_START_RATIO };
			const float shrink{ std::clamp((debris.m_elapsed - shrinkStart) /
				                               (debris.m_lifetime - shrinkStart),
				0.0f, 1.0f) };

			const auto* destructible{ m_componentManager.tryGet<component::stage::DestructibleComponent>(entityId) };
			if (destructible == nullptr)
				continue;

			for (auto& fragment : debris.m_fragments)
			{
				fragment.m_position += fragment.m_velocity * deltaTime;
				fragment.m_velocity.y -= GRAVITY * deltaTime;
				fragment.m_velocity = fragment.m_velocity * (1.0f - std::min(0.9f, AIR_DRAG * deltaTime));
				fragment.m_rotation += fragment.m_angular * deltaTime;

				if (fragment.m_position.y < debris.m_floorY)
				{
					fragment.m_position.y = debris.m_floorY;
					fragment.m_velocity.y = -fragment.m_velocity.y * BOUNCE_RETENTION;
					fragment.m_velocity.x *= FLOOR_FRICTION;
					fragment.m_velocity.z *= FLOOR_FRICTION;
					fragment.m_angular = fragment.m_angular * SPIN_DAMPING;
				}

				fragment.m_scale = 1.0f - shrink;

				m_renderer.setModelFrameTransform(destructible->m_fracturedHandle, fragment.m_frameIndex,
				    fragment.m_pivot, fragment.m_position, fragment.m_rotation,
				    debris.m_modelScale * fragment.m_scale);
			}
		}
	}
} // namespace game::system::stage
