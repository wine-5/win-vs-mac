#include "DetectionAlertVisualsSystem.h"
#include "game/component/AlertComponent.h"
#include "game/component/TransformComponent.h"
#include "game/component/ColliderComponent.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"
#include <algorithm>
#include <array>

namespace
{
	// バナーの表示時間（秒）
	constexpr float ALERT_DURATION{ 1.6f };
	// 出現時のフェードイン・せり上がりにかける時間（秒）
	constexpr float APPEAR_TIME{ 0.18f };
	// 表示終了間際にフェードアウトする時間（秒）
	constexpr float FADE_TIME{ 0.35f };
	// 出現時にせり上がる量（バナー高さ比）
	constexpr float RISE_RATIO{ 0.45f };

	// バナー高さ（画面高さ比。解像度非依存にするため）
	constexpr float BANNER_HEIGHT_RATIO{ 0.052f };
	// 頭上へバナー下端を浮かせるワールド単位のマージン
	constexpr float HEAD_MARGIN{ 45.0f };
	// コライダーが無い敵のフォールバック頭上高さ（ワールド単位）
	constexpr float FALLBACK_HEAD_HEIGHT{ 180.0f };

	// 危険メッセージ（発見時にランダムで1つ選ぶ）
	constexpr std::array<const char*, 3> ALERT_MESSAGES{ "危険！", "見つかった！", "発見！" };

	// 配色
	constexpr unsigned int BANNER_BG{ core::utility::Color::rgb(248, 248, 250) }; // iOS通知バナー風の明るい地
	constexpr unsigned int ICON_BG{ core::utility::Color::rgb(255, 228, 228) };   // アイコンの明るい赤（薄）
	constexpr unsigned int DANGER_RED{ core::utility::Color::rgb(224, 36, 36) };  // 「！」とメッセージの赤
} // namespace

namespace game::system
{
	DetectionAlertVisualsSystem::DetectionAlertVisualsSystem(core::ecs::ComponentManager& componentManager,
	    core::base::EventBus& eventBus,
	    core::iface::IRenderer& renderer,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen)
	    : m_componentManager{ componentManager }
	    , m_eventBus{ eventBus }
	    , m_renderer{ renderer }
	    , m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	{
		m_eventBus.subscribe<event::EnemyAlertedEvent>(
		    [this](const event::EnemyAlertedEvent& e)
		    { onEnemyAlerted(e); });
	}

	void DetectionAlertVisualsSystem::onEnemyAlerted(const event::EnemyAlertedEvent& e)
	{
		std::uniform_int_distribution<std::size_t> dist{ 0, ALERT_MESSAGES.size() - 1 };
		const int messageIndex{ static_cast<int>(dist(m_rng)) };

		// 既に表示中なら時間を延ばしつつ文言も引き直す。無ければ付与する
		if (m_componentManager.has<component::AlertComponent>(e.m_entityId))
		{
			auto& alert{ m_componentManager.get<component::AlertComponent>(e.m_entityId) };
			alert.m_timer = ALERT_DURATION;
			alert.m_messageIndex = messageIndex;
		}
		else
		{
			m_componentManager.add<component::AlertComponent>(e.m_entityId,
			    component::AlertComponent{ ALERT_DURATION, messageIndex });
		}
	}

	void DetectionAlertVisualsSystem::update(float deltaTime)
	{
		// getAllEntitiesはスナップショットを返すため、ループ中の削除は安全
		auto entities{ m_componentManager.getAllEntities<component::AlertComponent>() };
		for (auto entityId : entities)
		{
			auto& alert{ m_componentManager.get<component::AlertComponent>(entityId) };
			alert.m_timer -= deltaTime;
			if (alert.m_timer <= 0.0f)
				m_componentManager.remove<component::AlertComponent>(entityId);
		}
	}

	void DetectionAlertVisualsSystem::draw()
	{
		const float screenH{ static_cast<float>(m_screen.getHeight()) };
		const float bannerH{ screenH * BANNER_HEIGHT_RATIO };

		auto entities{ m_componentManager.getAllEntities<component::AlertComponent>() };
		for (auto entityId : entities)
		{
			if (!m_componentManager.has<component::TransformComponent>(entityId))
				continue;

			const auto& alert{ m_componentManager.get<component::AlertComponent>(entityId) };
			const auto& transform{ m_componentManager.get<component::TransformComponent>(entityId) };

			// 頭上のワールド座標を求める（原点は足元。コライダー高さぶん上へ）
			float headHeight{ FALLBACK_HEAD_HEIGHT };
			if (m_componentManager.has<component::ColliderComponent>(entityId))
				headHeight = m_componentManager.get<component::ColliderComponent>(entityId).m_size.y;

			core::Vector3 headWorld{ transform.m_position };
			headWorld.y += headHeight + HEAD_MARGIN;

			// スクリーン座標へ投影。カメラ後方・画面外（深度が0〜1の外）なら描かない
			const core::Vector3 screen{ m_renderer.worldToScreen(headWorld) };
			if (screen.z < 0.0f || screen.z > 1.0f)
				continue;

			const float elapsed{ ALERT_DURATION - alert.m_timer };

			// 出現直後は少し下からせり上がってくる
			const float appearProgress{ std::clamp(elapsed / APPEAR_TIME, 0.0f, 1.0f) };
			const float riseOffset{ (1.0f - appearProgress) * bannerH * RISE_RATIO };

			// フェードイン（出現時）とフェードアウト（終了間際）の小さい方を採用する
			const float fadeIn{ appearProgress };
			const float fadeOut{ alert.m_timer < FADE_TIME ? std::clamp(alert.m_timer / FADE_TIME, 0.0f, 1.0f) : 1.0f };
			const float alpha{ std::min(fadeIn, fadeOut) };

			// バナー下端が頭のすぐ上に来るように上端Yを決める
			const int centerX{ static_cast<int>(screen.x) };
			const int topY{ static_cast<int>(screen.y - bannerH + riseOffset) };

			const int alphaParam{ static_cast<int>(255.0f * alpha) };
			m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alphaParam);
			drawBanner(centerX, topY, ALERT_MESSAGES[alert.m_messageIndex]);
			m_uiRenderer.resetBlendMode();
		}
	}

	void DetectionAlertVisualsSystem::drawBanner(int centerX, int topY, const char* message)
	{
		const float screenH{ static_cast<float>(m_screen.getHeight()) };
		const int bannerH{ static_cast<int>(screenH * BANNER_HEIGHT_RATIO) };
		const int pad{ std::max(2, static_cast<int>(bannerH * 0.16f)) };
		const int iconSize{ bannerH - pad * 2 };
		const int msgFont{ std::max(8, static_cast<int>(bannerH * 0.40f)) };

		// メッセージ幅に合わせてバナー横幅を決める（左パディング・アイコン・間隔・文字・右パディング）
		const int textWidth{ m_uiRenderer.getTextWidth(message, msgFont) };
		const int bannerW{ pad + iconSize + pad + textWidth + pad };

		const int x{ centerX - bannerW / 2 };
		const int y{ topY };
		const int radius{ std::max(2, static_cast<int>(bannerH * 0.28f)) };

		// 背景（明るい角丸バー）
		drawRoundedRect(x, y, bannerW, bannerH, radius, BANNER_BG);

		// 左のアプリアイコン風の角丸四角（赤い縁取り＋薄赤地）
		const int iconX{ x + pad };
		const int iconY{ y + pad };
		const int iconRadius{ std::max(2, static_cast<int>(iconSize * 0.28f)) };
		const int borderInset{ std::max(1, iconSize / 12) };
		drawRoundedRect(iconX, iconY, iconSize, iconSize, iconRadius, DANGER_RED);
		drawRoundedRect(iconX + borderInset, iconY + borderInset,
		    iconSize - borderInset * 2, iconSize - borderInset * 2, iconRadius, ICON_BG);

		// アイコン内に赤い「！」を中央寄せで描く
		const int iconFont{ std::max(8, static_cast<int>(iconSize * 0.66f)) };
		const int exWidth{ m_uiRenderer.getTextWidth("!", iconFont) };
		m_uiRenderer.drawText(iconX + (iconSize - exWidth) / 2, iconY + (iconSize - iconFont) / 2,
		    "!", DANGER_RED, iconFont);

		// 危険メッセージ（赤）をアイコンの右へ、縦中央で描く
		const int msgX{ iconX + iconSize + pad };
		const int msgY{ y + (bannerH - msgFont) / 2 };
		m_uiRenderer.drawText(msgX, msgY, message, DANGER_RED, msgFont);
	}

	void DetectionAlertVisualsSystem::drawRoundedRect(int x, int y, int width, int height, int radius, unsigned int color)
	{
		// 角丸は「十字の矩形2枚＋四隅の円」で近似する
		radius = std::clamp(radius, 0, std::min(width, height) / 2);

		m_uiRenderer.drawBox(x + radius, y, width - radius * 2, height, color, true);
		m_uiRenderer.drawBox(x, y + radius, width, height - radius * 2, color, true);

		if (radius > 0)
		{
			m_uiRenderer.drawCircle(x + radius, y + radius, radius, color, true, 0);
			m_uiRenderer.drawCircle(x + width - radius, y + radius, radius, color, true, 0);
			m_uiRenderer.drawCircle(x + radius, y + height - radius, radius, color, true, 0);
			m_uiRenderer.drawCircle(x + width - radius, y + height - radius, radius, color, true, 0);
		}
	}
} // namespace game::system
