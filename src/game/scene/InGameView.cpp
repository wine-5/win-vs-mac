#include "InGameView.h"
#include "core/utility/Color.h"
#include "game/component/TransformComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/AimComponent.h"
#include "game/component/ProjectileComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/PlayerChargeComponent.h"
#include "game/system/PlayerChargeVisualsSystem.h"
#include "game/system/MacAwakenEffectSystem.h"
#include "game/system/DetectionAlertVisualsSystem.h"
#include "game/system/AttackTelegraphVisualsSystem.h"
#include "game/system/TelegraphVisualsSystem.h"
#include "game/ui/debug/DebugGizmoView.h" // DEBUG: リリース時に削除
#include "game/ui/debug/DebugHUDView.h"   // DEBUG: リリース時に削除
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

		// 攻撃予兆（地面の攻撃範囲サークル）。地面の上・敵の足元に3Dで描く（3D描画フェーズ）
		if (m_attackTelegraphSystem)
			m_attackTelegraphSystem->draw();

		// 汎用攻撃予兆（TelegraphComponent駆動：円・扇）。ボスの溜め攻撃などを描く
		if (m_telegraphSystem)
			m_telegraphSystem->draw();

		// プレイヤー弾の見た目は実OSウィンドウ（ProjectileWindowSystem）が担うため描かない。
		// 敵のタブ弾など3Dモデルを持つ弾はここで回転描画する
		drawProjectileModels();

		// DEBUG: 当たり判定等のワールド空間デバッグ可視化（リリース時に削除）
		if (m_debugGizmoView)
			m_debugGizmoView->draw();

		// プレイヤーの溜め攻撃の演出（集中線）。描画内容はSystemが持ち、Viewは描画順だけを決める
		if (m_playerChargeVisualsSystem)
			m_playerChargeVisualsSystem->draw();

		// ボス覚醒の赤ビネット（画面全体の演出。HUDより奥に描く）
		if (m_macAwakenEffectSystem)
			m_macAwakenEffectSystem->draw();

		// 敵の発見演出（頭上の通知バッジ）。モデルの手前・HUDより奥に描く
		if (m_detectionAlertSystem)
			m_detectionAlertSystem->draw();

		// 照準レティクル（HUD）は最前面に描く
		drawReticle(playerId);

		// DEBUG: デバッグHUD（FPS等の統計・カメラ状態ラベル）（リリース時に削除）
		if (m_debugHUDView)
			m_debugHUDView->draw(static_cast<int>(enemyIds.size()));

		// Effekseerエフェクトの描画（3Dモデル描画後・UI手前に呼び出す）
		m_effectFactory.draw();
	}

	void InGameView::setPlayerChargeVisualsSystem(system::PlayerChargeVisualsSystem* system)
	{
		m_playerChargeVisualsSystem = system;
	}

	void InGameView::setMacAwakenEffectSystem(system::MacAwakenEffectSystem* system)
	{
		m_macAwakenEffectSystem = system;
	}

	void InGameView::setDetectionAlertVisualsSystem(system::DetectionAlertVisualsSystem* system)
	{
		m_detectionAlertSystem = system;
	}

	void InGameView::setAttackTelegraphVisualsSystem(system::AttackTelegraphVisualsSystem* system)
	{
		m_attackTelegraphSystem = system;
	}

	void InGameView::setTelegraphVisualsSystem(system::TelegraphVisualsSystem* system)
	{
		m_telegraphSystem = system;
	}

	void InGameView::setDebugGizmoView(ui::debug::DebugGizmoView* view)
	{
		m_debugGizmoView = view;
	}

	void InGameView::setDebugHUDView(ui::debug::DebugHUDView* view)
	{
		m_debugHUDView = view;
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

			if (projectile.m_spinRollSpeed > 0.0f)
			{
				// レインボーは飛ぶ方向に依らず常にカメラへ正対させ、面内で回す「ルーレット」。
				// 回転の速さ（m_spinRollSpeed）はmacData.jsonのrainbowSpinSpeedから渡される。
				const float roll{ distance * projectile.m_spinRollSpeed };
				// 進行方向（velocity）を向いたまま飛ばす。モデルの正面が逆なので反転して渡す（180度逆）
				core::Vector3 faceDir{ 0.0f, 0.0f, -1.0f };
				if (m_componentManager.has<component::VelocityComponent>(id))
				{
					const auto& vel{ m_componentManager.get<component::VelocityComponent>(id).m_velocity };
					faceDir = core::Vector3{ -vel.x, -vel.y, -vel.z };
				}
				m_renderer.drawSpinningModelFacing(render.m_modelHandle, transform.m_position,
				    transform.m_scale, projectile.m_spinCenter, faceDir, roll);
				continue;
			}

			// それ以外の弾（タブ等）は従来どおり左右にぐるぐる（Y軸まわり）回す
			const core::Vector3 rotation{ 0.0f, tumble, 0.0f };
			m_renderer.drawModel(render.m_modelHandle, transform.m_position, rotation, transform.m_scale);
		}
	}
} // namespace game::scene
