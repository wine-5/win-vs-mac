#include "DebugGizmoView.h"
#include "core/utility/Color.h"
#include "game/component/TransformComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/ai/AIComponent.h"
#include "game/component/EnemyTypeComponent.h"
#include "game/component/combat/ProjectileComponent.h"

namespace
{
	/**
	 * @brief 浮遊する敵（Safari）かを判定する
	 * @param componentManager ComponentManagerの参照
	 * @param id 判定対象のEntityId
	 * @return 浮遊型ならtrue
	 */
	bool isFlyingEnemyImpl(core::ecs::ComponentManager& componentManager, core::ecs::EntityId id)
	{
		const auto* type{ componentManager.tryGet<game::component::EnemyTypeComponent>(id) };
		return type != nullptr && type->m_type == game::constant::EnemyType::Safari;
	}
} // namespace

namespace game::ui::debug
{
	DebugGizmoView::DebugGizmoView(core::ecs::ComponentManager& componentManager,
	    core::iface::IRenderer& renderer)
	    : m_componentManager{ componentManager }
	    , m_renderer{ renderer }
	{
	}

	void DebugGizmoView::draw()
	{
		if (!m_isEnabled)
			return;

		if (m_isColliderEnabled)
			drawColliders();

		if (m_isAttackRangeEnabled)
			drawAttackRanges();

		if (m_isDetectionRangeEnabled)
			drawDetectionRanges();

		if (m_isProjectileRangeEnabled)
			drawProjectileRanges();
	}

	void DebugGizmoView::setEnabled(bool enabled)
	{
		m_isEnabled = enabled;
	}

	void DebugGizmoView::setColliderEnabled(bool enabled)
	{
		m_isColliderEnabled = enabled;
	}

	void DebugGizmoView::setAttackRangeEnabled(bool enabled)
	{
		m_isAttackRangeEnabled = enabled;
	}

	void DebugGizmoView::setDetectionRangeEnabled(bool enabled)
	{
		m_isDetectionRangeEnabled = enabled;
	}

	void DebugGizmoView::setProjectileRangeEnabled(bool enabled)
	{
		m_isProjectileRangeEnabled = enabled;
	}

	void DebugGizmoView::drawColliders()
	{
		auto colliderEntities{ m_componentManager.getAllEntities<component::combat::ColliderComponent>() };
		for (auto id : colliderEntities)
		{
			auto& colliderTf{ m_componentManager.get<component::TransformComponent>(id) };
			auto& collider{ m_componentManager.get<component::combat::ColliderComponent>(id) };
			core::Vector3 colliderCenter{ colliderTf.m_position + collider.m_offset };
			m_renderer.drawCollider(colliderCenter, collider.m_size, core::utility::Color::BLUE);
		}
	}

	void DebugGizmoView::drawAttackRanges()
	{
		// 人型（プレイヤー・Xcode・Mac）はカプセル、浮遊型ドローン（Safari）は球で描く
		auto attackers{ m_componentManager.getAllEntities<component::combat::AttackComponent>() };
		for (auto id : attackers)
		{
			auto& atkTransform{ m_componentManager.get<component::TransformComponent>(id) };
			auto& atk{ m_componentManager.get<component::combat::AttackComponent>(id) };

			// 浮遊型（Safari）はスポーン時に確定した敵種で判定する
			const bool isFlying{ isFlyingEnemyImpl(m_componentManager, id) };

			if (!isFlying && m_componentManager.has<component::combat::ColliderComponent>(id))
			{
				auto& collider{ m_componentManager.get<component::combat::ColliderComponent>(id) };
				core::Vector3 center{ atkTransform.m_position + collider.m_offset };
				float halfHeight{ collider.m_size.y * 0.5f };
				core::Vector3 bottom{ center.x, center.y - halfHeight, center.z };
				core::Vector3 top{ center.x, center.y + halfHeight, center.z };
				m_renderer.drawDebugCapsule(bottom, top, atk.m_attackRange, core::utility::Color::rgb(255, 0, 0));
			}
			else
			{
				m_renderer.drawDebugSphere(atkTransform.m_position, atk.m_attackRange, core::utility::Color::rgb(255, 0, 0));
			}
		}
	}

	void DebugGizmoView::drawDetectionRanges()
	{
		// 人型（プレイヤー・Xcode・Mac）はカプセル、浮遊型ドローン（Safari）は球で描く
		auto attackers{ m_componentManager.getAllEntities<component::combat::AttackComponent>() };
		for (auto id : attackers)
		{
			if (!m_componentManager.has<component::ai::AIComponent>(id))
				continue;

			auto& atkTransform{ m_componentManager.get<component::TransformComponent>(id) };
			auto& ai{ m_componentManager.get<component::ai::AIComponent>(id) };

			// 浮遊型（Safari）はスポーン時に確定した敵種で判定する
			const bool isFlying{ isFlyingEnemyImpl(m_componentManager, id) };
			if (!isFlying && m_componentManager.has<component::combat::ColliderComponent>(id))
			{
				auto& collider{ m_componentManager.get<component::combat::ColliderComponent>(id) };
				core::Vector3 center{ atkTransform.m_position + collider.m_offset };
				float halfHeight{ collider.m_size.y * 0.5f };
				core::Vector3 bottom{ center.x, center.y - halfHeight, center.z };
				core::Vector3 top{ center.x, center.y + halfHeight, center.z };
				m_renderer.drawDebugCapsule(bottom, top, ai.m_detectionRange, core::utility::Color::rgb(255, 255, 0));
			}
			else
				m_renderer.drawDebugSphere(atkTransform.m_position, ai.m_detectionRange, core::utility::Color::rgb(255, 255, 0));
		}
	}

	void DebugGizmoView::drawProjectileRanges()
	{
		auto projectiles{ m_componentManager.getAllEntities<component::combat::ProjectileComponent>() };
		for (auto id : projectiles)
		{
			if (!m_componentManager.has<component::combat::AttackComponent>(id))
				continue;

			const auto& transform{ m_componentManager.get<component::TransformComponent>(id) };
			const auto& atk{ m_componentManager.get<component::combat::AttackComponent>(id) };

			m_renderer.drawDebugSphere(transform.m_position, atk.m_attackRange, core::utility::Color::rgb(255, 0, 0));
		}
	}
} // namespace game::ui::debug
