#include "InGameView.h"
#include "core/utility/Color.h"
#include "game/component/TransformComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/ColliderComponent.h"
#include "game/component/AttackComponent.h"
#include "game/component/AIComponent.h"
#include "game/component/AimComponent.h"
#include "game/component/ProjectileComponent.h"

namespace game::scene
{
	InGameView::InGameView(core::ecs::ComponentManager& componentManager,
	    core::iface::IRenderer& renderer,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen)
	    : m_componentManager{ componentManager }
	    , m_renderer{ renderer }
	    , m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	{
	}

	void InGameView::draw(core::ecs::EntityId playerId,
	    core::ecs::EntityId groundId,
	    const std::vector<core::ecs::EntityId>& enemyIds)
	{
		drawModels(playerId, groundId, enemyIds);

		// プレイヤー弾の見た目は実OSウィンドウ（ProjectileWindowSystem）が担うため、
		// ゲーム内ビルボードは描かない（敵弾のビルボード実装時に再有効化する）
		// drawProjectiles();

		// DEBUG: デバッグ可視化（テスト後に呼び出しごと削除）
		drawDebugVisuals();

		// 照準レティクル（HUD）は最前面に描く
		drawReticle(playerId);
	}

	void InGameView::setDebugVisualsEnabled(bool enabled)
	{
		m_isDebugVisualsEnabled = enabled;
	}

	void InGameView::setDebugColliderEnabled(bool enabled)
	{
		m_isDebugColliderEnabled = enabled;
	}

	void InGameView::setDebugAttackRangeEnabled(bool enabled)
	{
		m_isDebugAttackRangeEnabled = enabled;
	}

	void InGameView::setDebugDetectionRangeEnabled(bool enabled)
	{
		m_isDebugDetectionRangeEnabled = enabled;
	}

	void InGameView::drawModels(core::ecs::EntityId playerId,
	    core::ecs::EntityId groundId,
	    const std::vector<core::ecs::EntityId>& enemyIds)
	{
		const auto& transform{ m_componentManager.get<component::TransformComponent>(playerId) };
		const auto& render{ m_componentManager.get<component::RenderComponent>(playerId) };
		const auto& groundTransform{ m_componentManager.get<component::TransformComponent>(groundId) };
		const auto& groundRender{ m_componentManager.get<component::RenderComponent>(groundId) };

		if (render.m_isVisible)
			m_renderer.drawModel(render.m_modelHandle, transform.m_position, transform.m_rotation, transform.m_scale);

		m_renderer.drawModel(groundRender.m_modelHandle, groundTransform.m_position, groundTransform.m_rotation, groundTransform.m_scale);

		for (auto enemyId : enemyIds)
		{
			const auto& enemyRender{ m_componentManager.get<component::RenderComponent>(enemyId) };
			const auto& enemyTransform{ m_componentManager.get<component::TransformComponent>(enemyId) };
			if (enemyRender.m_isVisible)
				m_renderer.drawModel(enemyRender.m_modelHandle, enemyTransform.m_position, enemyTransform.m_rotation, enemyTransform.m_scale);
		}
	}

	void InGameView::drawReticle(core::ecs::EntityId playerId)
	{
		// 敵を捕捉していれば赤、通常はティール
		bool onTarget{ false };
		if (m_componentManager.has<component::AimComponent>(playerId))
			onTarget = m_componentManager.get<component::AimComponent>(playerId).m_hasTarget;

		const unsigned int color{ onTarget
			                          ? core::utility::Color::rgb(255, 48, 48)    // 敵捕捉時は赤
			                          : core::utility::Color::rgb(0, 255, 180) }; // 通常はティール

		const int centerX{ m_screen.getWidth() / 2 };
		const int centerY{ m_screen.getHeight() / 2 };

		// 画面高さ基準でサイズを決め、解像度に依存しないようにする
		const int base{ m_screen.getHeight() };
		const int ringRadius{ static_cast<int>(base * 0.030f) };
		const int tickLength{ static_cast<int>(base * 0.018f) };
		const int gap{ ringRadius + static_cast<int>(base * 0.006f) };
		constexpr int THICKNESS{ 2 };
		constexpr int DOT_RADIUS{ 3 };
		const int halfThickness{ THICKNESS / 2 };

		// 外周リング
		m_uiRenderer.drawCircle(centerX, centerY, ringRadius, color, false, THICKNESS);
		// 上下左右のティック
		m_uiRenderer.drawBox(centerX - gap - tickLength, centerY - halfThickness, tickLength, THICKNESS, color, true);
		m_uiRenderer.drawBox(centerX + gap, centerY - halfThickness, tickLength, THICKNESS, color, true);
		m_uiRenderer.drawBox(centerX - halfThickness, centerY - gap - tickLength, THICKNESS, tickLength, color, true);
		m_uiRenderer.drawBox(centerX - halfThickness, centerY + gap, THICKNESS, tickLength, color, true);
		// 中心ドット
		m_uiRenderer.drawCircle(centerX, centerY, DOT_RADIUS, color, true, 1);
	}

	void InGameView::drawProjectiles()
	{
		auto projectiles{ m_componentManager.getAllEntities<component::ProjectileComponent>() };
		for (auto id : projectiles)
		{
			const auto& projectile{ m_componentManager.get<component::ProjectileComponent>(id) };
			const auto& transform{ m_componentManager.get<component::TransformComponent>(id) };
			float radius{ 40.0f };
			if (m_componentManager.has<component::AttackComponent>(id))
				radius = m_componentManager.get<component::AttackComponent>(id).m_attackRange;

			if (projectile.m_imageHandle != -1)
			{
				// 常にカメラを向くビルボードとして描画する（一辺＝直径×スケール）
				const float size{ radius * 2.0f * transform.m_scale.x };
				m_renderer.drawBillboard(projectile.m_imageHandle, transform.m_position, size);
			}
			else
			{
				// 画像未設定時の仮描画：ティールのスフィア
				m_renderer.drawDebugSphere(transform.m_position, radius, core::utility::Color::rgb(0, 255, 180));
			}
		}
	}

	void InGameView::drawDebugVisuals()
	{
		if (!m_isDebugVisualsEnabled)
			return;

		if (m_isDebugColliderEnabled)
			drawDebugColliders();

		if (m_isDebugAttackRangeEnabled)
			drawDebugAttackRanges();

		if (m_isDebugDetectionRangeEnabled)
			drawDebugDetectionRanges();
	}

	void InGameView::drawDebugColliders()
	{
		auto colliderEntities{ m_componentManager.getAllEntities<component::ColliderComponent>() };
		for (auto id : colliderEntities)
		{
			auto& colliderTf{ m_componentManager.get<component::TransformComponent>(id) };
			auto& collider{ m_componentManager.get<component::ColliderComponent>(id) };
			core::Vector3 colliderCenter{ colliderTf.m_position + collider.m_offset };
			m_renderer.drawCollider(colliderCenter, collider.m_size, core::utility::Color::BLUE);
		}
	}

	void InGameView::drawDebugAttackRanges()
	{
		// 人型（プレイヤー・Xcode・Mac）はカプセル、浮遊型ドローン（Safari）は球で描く
		auto attackers{ m_componentManager.getAllEntities<component::AttackComponent>() };
		for (auto id : attackers)
		{
			auto& atkTransform{ m_componentManager.get<component::TransformComponent>(id) };
			auto& atk{ m_componentManager.get<component::AttackComponent>(id) };

			bool isFlying{ false };
			if (m_componentManager.has<component::AIComponent>(id))
			{
				auto& ai{ m_componentManager.get<component::AIComponent>(id) };
				isFlying = (ai.m_behavior == constant::AIBehavior::RangeKeepDistance);
			}

			if (!isFlying && m_componentManager.has<component::ColliderComponent>(id))
			{
				auto& collider{ m_componentManager.get<component::ColliderComponent>(id) };
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

	void InGameView::drawDebugDetectionRanges()
	{
		// 人型（プレイヤー・Xcode・Mac）はカプセル、浮遊型ドローン（Safari）は球で描く
		auto attackers{ m_componentManager.getAllEntities<component::AttackComponent>() };
		for (auto id : attackers)
		{
			if (!m_componentManager.has<component::AIComponent>(id))
				continue;

			auto& atkTransform{ m_componentManager.get<component::TransformComponent>(id) };
			auto& ai{ m_componentManager.get<component::AIComponent>(id) };

			const bool isFlying{ ai.m_behavior == constant::AIBehavior::RangeKeepDistance };
			if (!isFlying && m_componentManager.has<component::ColliderComponent>(id))
			{
				auto& collider{ m_componentManager.get<component::ColliderComponent>(id) };
				core::Vector3 center{ atkTransform.m_position + collider.m_offset };
				float halfHeight{ collider.m_size.y * 0.5f };
				core::Vector3 bottom{ center.x, center.y - halfHeight, center.z };
				core::Vector3 top{ center.x, center.y + halfHeight, center.z };
				m_renderer.drawDebugCapsule(bottom, top, ai.m_detectionRange, core::utility::Color::rgb(255, 255, 0));
			}
			else
			{
				m_renderer.drawDebugSphere(atkTransform.m_position, ai.m_detectionRange, core::utility::Color::rgb(255, 255, 0));
			}
		}
	}
} // namespace game::scene
