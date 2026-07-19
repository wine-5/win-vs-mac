#include "InGameView.h"
#include "core/utility/Color.h"
#include "game/component/TransformComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/ColliderComponent.h"
#include "game/component/AttackComponent.h"
#include "game/component/AIComponent.h"
#include "game/component/AimComponent.h"
#include "game/component/ProjectileComponent.h"
#include "game/component/PlayerChargeComponent.h"
#include "game/system/PlayerChargeVisualsSystem.h"
#include <cmath>

namespace game::scene
{
	InGameView::InGameView(core::ecs::ComponentManager& componentManager,
	    core::iface::IRenderer& renderer,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::iface::IEffectFactory& effectFactory)
	    : m_componentManager{ componentManager }
	    , m_renderer{ renderer }
	    , m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_effectFactory{ effectFactory }
	{
	}

	void InGameView::draw(core::ecs::EntityId playerId,
	    core::ecs::EntityId groundId,
	    const std::vector<core::ecs::EntityId>& enemyIds)
	{
		drawModels(playerId, groundId, enemyIds);

		// プレイヤー弾の見た目は実OSウィンドウ（ProjectileWindowSystem）が担うため描かない。
		// 敵のタブ弾など3Dモデルを持つ弾はここで回転描画する
		drawProjectileModels();

		// DEBUG: デバッグ可視化（テスト後に呼び出しごと削除）
		drawDebugVisuals();

		// プレイヤーの溜め攻撃の演出（集中線）。描画内容はSystemが持ち、Viewは描画順だけを決める
		if (m_playerChargeVisualsSystem)
			m_playerChargeVisualsSystem->draw();

		// 照準レティクル（HUD）は最前面に描く
		drawReticle(playerId);

		// Effekseerエフェクトの描画（3Dモデル描画後・UI手前に呼び出す）
		// m_effectFactory.draw();
	}

	void InGameView::setPlayerChargeVisualsSystem(system::PlayerChargeVisualsSystem* system)
	{
		m_playerChargeVisualsSystem = system;
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

	void InGameView::setDebugProjectileRangeEnabled(bool enabled)
	{
		m_isDebugProjectileRangeEnabled = enabled;
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
		// 敵を捕捉していれば赤、最大溜め完了ならWindowsロゴの水色、通常は黒
		// （捕捉＝発射判断に直結する情報なので最優先で表示する）
		bool onTarget{ false };
		if (m_componentManager.has<component::AimComponent>(playerId))
			onTarget = m_componentManager.get<component::AimComponent>(playerId).m_hasTarget;

		bool isMaxCharged{ false };
		if (m_componentManager.has<component::PlayerChargeComponent>(playerId))
		{
			const auto& charge{ m_componentManager.get<component::PlayerChargeComponent>(playerId) };
			isMaxCharged = charge.m_isCharging && charge.m_chargeRate >= 1.0f;
		}

		unsigned int color{ core::utility::Color::BLACK };
		if (onTarget)
			color = core::utility::Color::rgb(255, 48, 48);
		else if (isMaxCharged)
			color = core::utility::Color::WINDOWS_LOGO_BLUE;

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

	void InGameView::drawProjectileModels()
	{
		// 発射地点からの移動距離に掛ける係数（1ワールド単位あたりのタンブル回転量[rad]）
		constexpr float TUMBLE_PER_UNIT{ 0.015f };

		auto projectiles{ m_componentManager.getAllEntities<component::ProjectileComponent>() };
		for (auto id : projectiles)
		{
			if (!m_componentManager.has<component::RenderComponent>(id))
				continue;

			const auto& render{ m_componentManager.get<component::RenderComponent>(id) };
			if (!render.m_isVisible || render.m_modelHandle == -1)
				continue;

			const auto& transform{ m_componentManager.get<component::TransformComponent>(id) };
			const auto& projectile{ m_componentManager.get<component::ProjectileComponent>(id) };

			// 発射地点からの移動距離に応じてタンブルさせる（状態を持たず距離から導出する）
			const core::Vector3 traveled{
				transform.m_position.x - projectile.m_spawnPosition.x,
				transform.m_position.y - projectile.m_spawnPosition.y,
				transform.m_position.z - projectile.m_spawnPosition.z
			};
			const float distance{ std::sqrt(traveled.x * traveled.x + traveled.y * traveled.y + traveled.z * traveled.z) };
			const float tumble{ distance * TUMBLE_PER_UNIT };

			// 左右にぐるぐる回す（垂直＝Y軸まわりの回転）
			const core::Vector3 rotation{ 0.0f, tumble, 0.0f };
			m_renderer.drawModel(render.m_modelHandle, transform.m_position, rotation, transform.m_scale);
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

		if (m_isDebugProjectileRangeEnabled)
			drawDebugProjectileRanges();
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
				m_renderer.drawDebugSphere(atkTransform.m_position, ai.m_detectionRange, core::utility::Color::rgb(255, 255, 0));
		}
	}

	void InGameView::drawDebugProjectileRanges()
	{
		auto projectiles{ m_componentManager.getAllEntities<component::ProjectileComponent>() };
		for (auto id : projectiles)
		{
			if (!m_componentManager.has<component::AttackComponent>(id))
				continue;

			const auto& transform{ m_componentManager.get<component::TransformComponent>(id) };
			const auto& atk{ m_componentManager.get<component::AttackComponent>(id) };

			m_renderer.drawDebugSphere(transform.m_position, atk.m_attackRange, core::utility::Color::rgb(255, 0, 0));
		}
	}

} // namespace game::scene
