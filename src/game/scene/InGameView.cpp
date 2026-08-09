#include "InGameView.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/visual/RenderComponent.h"
#include "game/component/stage/ExtensionPickupComponent.h"
#include "game/component/visual/WeaponAttachComponent.h"
#include "game/component/combat/AimComponent.h"
#include "game/component/combat/ProjectileComponent.h"
#include "game/component/combat/DeathComponent.h"
#include "game/component/ai/AIComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/combat/PlayerChargeComponent.h"
#include "game/system/visual/PlayerChargeVisualsSystem.h"
#include "game/system/visual/CriticalVisualsSystem.h"
#include "game/system/visual/MacAwakenEffectSystem.h"
#include "game/system/visual/DetectionAlertVisualsSystem.h"
#include "game/system/visual/DamagePopupSystem.h"
#include "game/system/visual/AttackTelegraphVisualsSystem.h"
#include "game/system/visual/TelegraphVisualsSystem.h"
#include "game/system/visual/BackgroundParticleSystem.h"
#include "game/system/visual/HardAuraVisualsSystem.h"
#include "game/system/visual/BattleStartSystem.h"
#include "game/system/combat/PlayerDeathSystem.h"
#include "game/system/combat/PlayerRangedAttackSystem.h"
#include "game/component/combat/AttackComponent.h"
#include "game/ui/debug/DebugGizmoView.h" // DEBUG: リリース時に削除
#include "game/ui/debug/DebugHUDView.h"   // DEBUG: リリース時に削除
#include "game/ui/ingame/PlayerHUDView.h"
#include "game/ui/ingame/EquipmentSlotView.h"
#include "game/ui/ingame/ObjectiveView.h"
#include "game/ui/ingame/InGameStatusView.h"
#include "game/ui/ingame/InventoryView.h"
#include "game/ui/ingame/LowHealthVignetteView.h"
#include "game/ui/ingame/BossHUDView.h"
#include "game/ui/ingame/MiniMapView.h"
#include "game/ui/ingame/EnemyHealthBarView.h"
#include <algorithm>
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

	void InGameView::draw(core::ecs::EntityId playerId, int remainingEnemyCount, core::ecs::EntityId bossId,
	    float elapsedTime)
	{
		// 虚空を流れるデータの光跡。壁や床に隠れてほしいのでモデルと同じ3D描画フェーズで、
		// かつ最初に描いて他の要素の背景に回す
		if (m_backgroundParticleSystem)
			m_backgroundParticleSystem->draw();

		drawModels();

		// Hardの敵を包む赤いオーラ。敵モデルの直後に重ねて「体から漏れる光」に見せる
		if (m_hardAuraVisualsSystem)
			m_hardAuraVisualsSystem->draw();

		// 攻撃予兆（地面の攻撃範囲サークル）。地面の上・敵の足元に3Dで描く（3D描画フェーズ）
		if (m_attackTelegraphSystem)
			m_attackTelegraphSystem->draw();

		// 汎用攻撃予兆（TelegraphComponent駆動：円・扇）。ボスの溜め攻撃などを描く
		if (m_telegraphSystem)
			m_telegraphSystem->draw();

		// 弾を描く。プレイヤーのWindow弾はビルボード、敵のタブ弾など3Dモデルはモデルで描画する
		drawProjectileModels();

		// 壊したブロックから落ちた拡張子の欠片。壁の裏では隠れてほしいので3D描画フェーズで描く
		drawExtensionPickups();

		// DEBUG: 当たり判定等のワールド空間デバッグ可視化（リリース時に削除）
		if (m_debugGizmoView)
			m_debugGizmoView->draw();

		// プレイヤーの溜め攻撃の演出（集中線）。描画内容はSystemが持ち、Viewは描画順だけを決める
		if (m_playerChargeVisualsSystem)
			m_playerChargeVisualsSystem->draw();

		// クリティカルの集中線。溜めの集中線と同じ層に、その手前で重ねる
		if (m_criticalVisualsSystem)
			m_criticalVisualsSystem->draw();

		// ボス覚醒の赤ビネット（画面全体の演出。HUDより奥に描く）
		if (m_macAwakenEffectSystem)
			m_macAwakenEffectSystem->draw();

		// 敵の発見演出（頭上の通知バッジ）。モデルの手前・HUDより奥に描く
		if (m_detectionAlertSystem)
			m_detectionAlertSystem->draw();

		// 敵の頭上HPバー。同じ頭上に出る発見バッジより手前に描く
		if (m_enemyHealthBarView)
			m_enemyHealthBarView->draw(bossId);

		// ダメージ数値。HPバーと重なる位置に出るため、必ず読めるようその手前に描く
		if (m_damagePopupSystem)
			m_damagePopupSystem->draw();

		// インベントリを開いている間は常設のHUDを描かない。
		// インベントリの下地は半透明なので、そのまま描くとパネルや光の帯が
		// 透けて重なり、読ませたい内容の上にノイズが乗る
		if (!m_isInventoryOpen)
		{
			// プレイヤーステータス（左下のHP）。演出より手前・レティクルと同じHUD層に描く
			if (m_playerHUDView)
				m_playerHUDView->draw(playerId);

			// 装備スロット（右下）
			if (m_equipmentSlotView)
				m_equipmentSlotView->draw();

			// 目標（左上）。開始演出のミッションが中央から流れ着くまでは伏せておく
			// （同じ内容が中央と左上に同時に出ていると、どちらを見ればよいのか分からない）
			if (m_objectiveView &&
			    (m_battleStartSystem == nullptr || m_battleStartSystem->isObjectiveRevealed()))
				m_objectiveView->draw(remainingEnemyCount, bossId != core::ecs::INVALID_ENTITY_ID);

			// 難易度と経過時間（右上）
			if (m_statusView)
				m_statusView->draw(elapsedTime);

			// ボスHP（上中央）。出現していなければ描かれない
			if (m_bossHUDView)
				m_bossHUDView->draw(bossId);

			// ミニマップ（右上・難易度パネルの下）
			if (m_miniMapView)
				m_miniMapView->draw(playerId);

			// 低HP警告のビネット。四隅を赤く染めるが、下の隅はHUDのパネルが占めているため、
			// パネルより手前に描かないと下2つの隅が隠れてしまう。
			// 画面全体が危険な状態なので、HUDごと赤く染まるほうが表現としても正しい
			if (m_lowHealthVignetteView)
				m_lowHealthVignetteView->draw(playerId);

			// 照準レティクル（HUD）は最前面に描く
			drawReticle(playerId);
		}

		// DEBUG: デバッグHUD（FPS等の統計・カメラ状態ラベル）（リリース時に削除）
		// 敵数はAIComponentを持つEntity数から数える（IDリストを引き回さない）
		if (m_debugHUDView)
			m_debugHUDView->draw(
			    static_cast<int>(m_componentManager.getAllEntities<component::ai::AIComponent>().size()));

		// Effekseerエフェクトの描画（3Dモデル描画後・UI手前に呼び出す）
		m_effectFactory.draw();

		// 開始演出（READY / FIGHT!）。この間は操作できないので、HUDより手前に大きく出して
		// 「まだ始まっていない」ことを画面の中心で伝える
		if (m_battleStartSystem)
			m_battleStartSystem->draw();

		// インベントリ。画面を覆うので他のHUDより手前に描く。
		// ただし死亡の暗転よりは奥（死んだ瞬間に持ち物が前面に残ると締まらない）
		if (m_isInventoryOpen && m_inventoryView)
			m_inventoryView->draw(playerId);

		// プレイヤー死亡時の暗転。画面の全てを覆って暗くするため最後に描く
		if (m_playerDeathSystem)
			m_playerDeathSystem->draw();
	}

	void InGameView::setPlayerChargeVisualsSystem(system::visual::PlayerChargeVisualsSystem* system)
	{
		m_playerChargeVisualsSystem = system;
	}

	void InGameView::setCriticalVisualsSystem(system::visual::CriticalVisualsSystem* system)
	{
		m_criticalVisualsSystem = system;
	}

	void InGameView::setMacAwakenEffectSystem(system::visual::MacAwakenEffectSystem* system)
	{
		m_macAwakenEffectSystem = system;
	}

	void InGameView::setDetectionAlertVisualsSystem(system::visual::DetectionAlertVisualsSystem* system)
	{
		m_detectionAlertSystem = system;
	}

	void InGameView::setDamagePopupSystem(system::visual::DamagePopupSystem* system)
	{
		m_damagePopupSystem = system;
	}

	void InGameView::setAttackTelegraphVisualsSystem(system::visual::AttackTelegraphVisualsSystem* system)
	{
		m_attackTelegraphSystem = system;
	}

	void InGameView::setTelegraphVisualsSystem(system::visual::TelegraphVisualsSystem* system)
	{
		m_telegraphSystem = system;
	}

	void InGameView::setBackgroundParticleSystem(system::visual::BackgroundParticleSystem* system)
	{
		m_backgroundParticleSystem = system;
	}

	void InGameView::setHardAuraVisualsSystem(system::visual::HardAuraVisualsSystem* system)
	{
		m_hardAuraVisualsSystem = system;
	}

	void InGameView::setBattleStartSystem(system::visual::BattleStartSystem* system)
	{
		m_battleStartSystem = system;
	}

	void InGameView::setPlayerDeathSystem(system::combat::PlayerDeathSystem* system)
	{
		m_playerDeathSystem = system;
	}

	void InGameView::setPlayerRangedAttackSystem(system::combat::PlayerRangedAttackSystem* system)
	{
		m_playerRangedAttackSystem = system;
	}

	void InGameView::setDebugGizmoView(ui::debug::DebugGizmoView* view)
	{
		m_debugGizmoView = view;
	}

	void InGameView::setDebugHUDView(ui::debug::DebugHUDView* view)
	{
		m_debugHUDView = view;
	}

	void InGameView::setPlayerHUDView(ui::ingame::PlayerHUDView* view)
	{
		m_playerHUDView = view;
	}

	void InGameView::setEquipmentSlotView(ui::ingame::EquipmentSlotView* view)
	{
		m_equipmentSlotView = view;
	}

	void InGameView::setInGameStatusView(ui::ingame::InGameStatusView* view)
	{
		m_statusView = view;
	}

	void InGameView::setInventoryView(ui::ingame::InventoryView* view)
	{
		m_inventoryView = view;
	}

	void InGameView::setInventoryOpen(bool isOpen)
	{
		m_isInventoryOpen = isOpen;
	}

	void InGameView::setObjectiveView(ui::ingame::ObjectiveView* view)
	{
		m_objectiveView = view;
	}

	void InGameView::setLowHealthVignetteView(ui::ingame::LowHealthVignetteView* view)
	{
		m_lowHealthVignetteView = view;
	}

	void InGameView::setMiniMapView(ui::ingame::MiniMapView* view)
	{
		m_miniMapView = view;
	}

	void InGameView::setBossHUDView(ui::ingame::BossHUDView* view)
	{
		m_bossHUDView = view;
	}

	void InGameView::setEnemyHealthBarView(ui::ingame::EnemyHealthBarView* view)
	{
		m_enemyHealthBarView = view;
	}

	void InGameView::drawModels()
	{
		// RenderComponentを持つEntityを一律に描く。
		// プレイヤー・地面・敵をIDで名指ししないため、描画対象が増えてもここは変わらない。
		//
		// ただし2パスに分けている。死亡ディゾルブ中の敵は半透明合成で描かれ、
		// 半透明はZバッファがあっても描画順に依存するため、不透明を全て描いた後に回す。
		const auto entities{ m_componentManager.getAllEntities<component::visual::RenderComponent>() };

		for (const bool dissolvingPass : { false, true })
		{
			for (const auto entityId : entities)
			{
				const auto& render{ m_componentManager.get<component::visual::RenderComponent>(entityId) };
				if (!render.m_isVisible || render.m_modelHandle == -1)
					continue;

				// 弾は回転・向きの扱いが特殊なため drawProjectileModels が専用に描く
				if (m_componentManager.has<component::combat::ProjectileComponent>(entityId))
					continue;

				// 死亡演出中（＝半透明）かどうかで描くパスを振り分ける
				const bool isDissolving{ m_componentManager.has<component::combat::DeathComponent>(entityId) };
				if (isDissolving != dissolvingPass)
					continue;

				const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(entityId) };
				// 同じモデルをサイズ違い・流し方違いで使い回すため、貼り方は描画のたびに設定する。
				// 触らないモデル（キャラクター等）はそのまま。スキニングされたモデルへ
				// テクスチャ座標変換を掛けると不正なフレーム指定になり得るため
				const bool needsUv{ render.m_uvScaleU != 1.0f || render.m_uvScaleV != 1.0f ||
					                render.m_scrollOffsetU != 0.0f || render.m_scrollOffsetV != 0.0f };
				if (needsUv)
					m_renderer.setTextureScroll(render.m_modelHandle, render.m_uvScaleU, render.m_uvScaleV,
					    render.m_scrollOffsetU, render.m_scrollOffsetV);
				m_renderer.drawModel(render.m_modelHandle, transform.m_position, transform.m_rotation, transform.m_scale);

				// 装着武器は本体を描いた直後に描く。ボーンのワールド行列は本体の
				// 位置・回転・スケールが適用済みでなければ正しい姿勢にならない
				drawAttachedWeapon(entityId);
			}
		}
	}

	void InGameView::drawAttachedWeapon(core::ecs::EntityId entityId)
	{
		if (!m_componentManager.has<component::visual::WeaponAttachComponent>(entityId))
			return;

		const auto& attach{ m_componentManager.get<component::visual::WeaponAttachComponent>(entityId) };
		// フレーム番号の解決は WeaponAttachSystem が行う。未解決のうちは描かない
		if (!attach.m_isVisible || attach.m_modelHandle == -1 || attach.m_frameIndex < 0)
			return;

		const auto& render{ m_componentManager.get<component::visual::RenderComponent>(entityId) };
		m_renderer.drawModelOnFrame(attach.m_modelHandle, render.m_modelHandle, attach.m_frameIndex,
		    attach.m_offsetPosition, attach.m_offsetRotation, attach.m_offsetScale);
	}

	void InGameView::drawReticle(core::ecs::EntityId playerId)
	{
		// 敵を捕捉していれば赤、最大溜め完了ならシアン、通常は白
		// （捕捉＝発射判断に直結する情報なので最優先で表示する）
		bool onTarget{ false };
		if (m_componentManager.has<component::combat::AimComponent>(playerId))
			onTarget = m_componentManager.get<component::combat::AimComponent>(playerId).m_hasTarget;

		bool isMaxCharged{ false };
		if (m_componentManager.has<component::combat::PlayerChargeComponent>(playerId))
		{
			const auto& charge{ m_componentManager.get<component::combat::PlayerChargeComponent>(playerId) };
			isMaxCharged = charge.m_isCharging && charge.m_chargeRate >= 1.0f;
		}

		unsigned int color{ core::utility::Color::HUD_INK };
		if (onTarget)
			color = core::utility::Color::HUD_CRIT_RED;
		else if (isMaxCharged)
			color = core::utility::Color::HUD_CHARGE_CYAN;

		const int centerX{ m_screen.getWidth() / 2 };
		const int centerY{ m_screen.getHeight() / 2 };

		// 画面高さ基準でサイズを決め、解像度に依存しないようにする
		const int base{ m_screen.getHeight() };
		const int ringRadius{ static_cast<int>(base * 0.030f) };
		const int tickLength{ static_cast<int>(base * 0.018f) };
		// クールダウン中はティックを外へ開き、撃てるようになると閉じる。
		// 「今は撃てない」を形で示すので、視線を中央から動かさずに判断できる
		const float cooldownRatio{ getAttackCooldownRatio(playerId) };
		const int spread{ static_cast<int>(base * 0.022f * cooldownRatio) };
		const int gap{ ringRadius + static_cast<int>(base * 0.006f) + spread };
		constexpr int THICKNESS{ 2 };
		constexpr int DOT_RADIUS{ 3 };
		const int halfThickness{ THICKNESS / 2 };

		// 外周リングは白を薄く敷くだけに留める。中央の十字とドットだけが状態色で光り、
		// リングは「当たりの目安」として背景に溶ける（プロトタイプの rgba(255,255,255,.16) 相当）
		constexpr int RING_ALPHA{ 41 };
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, RING_ALPHA);
		m_uiRenderer.drawCircle(centerX, centerY, ringRadius, core::utility::Color::WHITE, false, THICKNESS);
		m_uiRenderer.resetBlendMode();
		// 上下左右のティック
		m_uiRenderer.drawBox(centerX - gap - tickLength, centerY - halfThickness, tickLength, THICKNESS, color, true);
		m_uiRenderer.drawBox(centerX + gap, centerY - halfThickness, tickLength, THICKNESS, color, true);
		m_uiRenderer.drawBox(centerX - halfThickness, centerY - gap - tickLength, THICKNESS, tickLength, color, true);
		m_uiRenderer.drawBox(centerX - halfThickness, centerY + gap, THICKNESS, tickLength, color, true);
		// 中心ドット
		m_uiRenderer.drawCircle(centerX, centerY, DOT_RADIUS, color, true, 1);

		// 溜めの進行度は外周リングに重ねる（視線を動かさずに撃ち時を判断できるようにする）
		drawChargeGauge(playerId, centerX, centerY, ringRadius);
	}

	float InGameView::getAttackCooldownRatio(core::ecs::EntityId playerId) const
	{
		float ratio{ 0.0f };

		// 近接はAttackComponentが残り時間を持つ
		if (m_componentManager.has<component::combat::AttackComponent>(playerId))
		{
			const auto& attack{ m_componentManager.get<component::combat::AttackComponent>(playerId) };
			if (attack.m_attackCooldown > 0.0f)
				ratio = std::clamp(attack.m_currentCooldown / attack.m_attackCooldown, 0.0f, 1.0f);
		}

		// 遠隔はSystemが内部で持つため、公開されている割合を使う
		if (m_playerRangedAttackSystem)
			ratio = std::max(ratio, m_playerRangedAttackSystem->getCooldownRatio());

		return ratio;
	}

	void InGameView::drawChargeGauge(core::ecs::EntityId playerId, int centerX, int centerY, int radius)
	{
		if (!m_componentManager.has<component::combat::PlayerChargeComponent>(playerId))
			return;

		// 溜めていないときは何も出さない。常時表示するとレティクル周りが常に賑やかになり、
		// 「溜まってきた」という変化そのものが読み取りにくくなる
		const auto& charge{ m_componentManager.get<component::combat::PlayerChargeComponent>(playerId) };
		if (!charge.m_isCharging)
			return;

		// 円弧を描くプリミティブが無いため、外周に点を並べて進行度を表す
		constexpr int DOT_COUNT{ 24 };
		constexpr int TRACK_ALPHA{ 46 }; // 未点灯の点（溜めの全体量を示す目盛り）
		constexpr float TWO_PI{ 6.283185f };
		constexpr float QUARTER_TURN{ 1.570796f }; // 真上を起点にするための回転量

		const int dotRadius{ std::max(2, static_cast<int>(m_screen.getHeight() * 0.0028f)) };
		const float rate{ std::clamp(charge.m_chargeRate, 0.0f, 1.0f) };
		const int litCount{ static_cast<int>(rate * DOT_COUNT) };

		// 未点灯ぶんを薄く敷いてから、点灯ぶんを不透明で上書きする（ブレンド切り替えを1回に抑える）
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, TRACK_ALPHA);
		for (int i{ litCount }; i < DOT_COUNT; ++i)
		{
			const float angle{ i * (TWO_PI / DOT_COUNT) - QUARTER_TURN };
			const int x{ centerX + static_cast<int>(std::cos(angle) * radius) };
			const int y{ centerY + static_cast<int>(std::sin(angle) * radius) };
			m_uiRenderer.drawCircle(x, y, dotRadius, core::utility::Color::WHITE, true, 1);
		}
		m_uiRenderer.resetBlendMode();

		// 最大まで溜まったら黄色へ振り切らせ、シアンのままの「溜め途中」と一目で区別できるようにする
		const unsigned int litColor{ rate >= 1.0f ? core::utility::Color::HUD_CHARGE_MAX
			                                      : core::utility::Color::HUD_CHARGE_CYAN };
		for (int i{ 0 }; i < litCount; ++i)
		{
			const float angle{ i * (TWO_PI / DOT_COUNT) - QUARTER_TURN };
			const int x{ centerX + static_cast<int>(std::cos(angle) * radius) };
			const int y{ centerY + static_cast<int>(std::sin(angle) * radius) };
			m_uiRenderer.drawCircle(x, y, dotRadius, litColor, true, 1);
		}
	}

	void InGameView::drawExtensionPickups()
	{
		// 拾えるものだと分かるよう、ゆっくり明滅させる
		constexpr float PULSE_SPEED{ 3.4f };
		constexpr int BRIGHTNESS_BASE{ 55 };
		constexpr int BRIGHTNESS_SWING{ 35 };

		// 足元に置く目印（遠くからでも「そこに何かある」と分かるように）。
		// 色はHUDの溜めと同じシアンで、床に敷くので薄く透かす
		constexpr float MARKER_RADIUS{ 46.0f };
		constexpr int MARKER_ALPHA{ 0x50 };
		constexpr unsigned int MARKER_COLOR{
			(core::utility::Color::HUD_CHARGE_CYAN & 0x00FFFFFFu) | (MARKER_ALPHA << 24)
		};
		constexpr float MARKER_HEIGHT{ 2.0f };

		const auto pickups{ m_componentManager.getAllEntities<component::stage::ExtensionPickupComponent>() };
		for (const auto id : pickups)
		{
			const auto* render{ m_componentManager.tryGet<component::visual::RenderComponent>(id) };
			if (render == nullptr || !render->m_isVisible || render->m_billboardImage == -1)
				continue;

			const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(id) };
			const auto& pickup{ m_componentManager.get<component::stage::ExtensionPickupComponent>(id) };

			// 足元の目印。浮いている欠片は床と離れていて位置が掴みにくいため、
			// 真下に円を描いて「どこに落ちているか」を示す
			m_renderer.drawGroundCircle({ transform.m_position.x, pickup.m_restY - MARKER_HEIGHT,
			                                transform.m_position.z },
			    MARKER_RADIUS, MARKER_COLOR, true);

			// まず通常合成で絵をそのまま描く。拡張子アイコンは暗い紙なので、
			// 加算合成だけで描くと暗い部分が何も足されず、ラベルの色しか見えない
			m_renderer.drawBillboard(render->m_billboardImage, transform.m_position,
			    render->m_billboardSize, 0.0f);

			// その上へ光を重ねて明滅させる。輪郭と記号だけが脈打ち、拾えるものだと分かる
			const float pulse{ std::sin(pickup.m_elapsed * PULSE_SPEED) };
			const int brightness{ BRIGHTNESS_BASE + static_cast<int>(pulse * BRIGHTNESS_SWING) };
			m_renderer.drawGlowBillboard(render->m_billboardImage, transform.m_position,
			    render->m_billboardSize, 0.0f, brightness);
		}
	}

	void InGameView::drawProjectileModels()
	{
		// 発射地点からの移動距離に掛ける係数（1ワールド単位あたりのタンブル回転量[rad]）
		constexpr float TUMBLE_PER_UNIT{ 0.015f };

		auto projectiles{ m_componentManager.getAllEntities<component::combat::ProjectileComponent>() };
		for (auto id : projectiles)
		{
			if (!m_componentManager.has<component::visual::RenderComponent>(id))
				continue;

			const auto& render{ m_componentManager.get<component::visual::RenderComponent>(id) };
			if (!render.m_isVisible)
				continue;

			const auto& transform{ m_componentManager.get<component::movement::TransformComponent>(id) };

			// プレイヤーのWindow弾はビルボード（板に貼ったWindow画像）で描く。
			// 深度を持つので壁の裏では隠れ、実OSウィンドウのようにプレイヤーを覆い隠さない
			if (render.m_billboardImage != -1)
			{
				m_renderer.drawBillboard(render.m_billboardImage, transform.m_position, render.m_billboardSize, 0.0f);
				continue;
			}

			if (render.m_modelHandle == -1)
				continue;

			const auto& projectile{ m_componentManager.get<component::combat::ProjectileComponent>(id) };

			// 発射地点からの移動距離に応じてタンブルさせる（状態を持たず距離から導出する）
			const float distance{ (transform.m_position - projectile.m_spawnPosition).length() };
			const float tumble{ distance * TUMBLE_PER_UNIT };

			if (projectile.m_spinRollSpeed > 0.0f)
			{
				// レインボーは飛ぶ方向に依らず常にカメラへ正対させ、面内で回す「ルーレット」。
				// 回転の速さ（m_spinRollSpeed）はmacData.jsonのrainbowSpinSpeedから渡される。
				const float roll{ distance * projectile.m_spinRollSpeed };
				// 進行方向（velocity）を向いたまま飛ばす。モデルの正面が逆なので反転して渡す（180度逆）
				core::Vector3 faceDir{ 0.0f, 0.0f, -1.0f };
				if (m_componentManager.has<component::movement::VelocityComponent>(id))
				{
					const auto& vel{ m_componentManager.get<component::movement::VelocityComponent>(id).m_velocity };
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
