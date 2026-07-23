#include "DetectionAlertVisualsSystem.h"
#include "game/component/EnemyTypeComponent.h"
#include "game/component/AlertComponent.h"
#include "game/component/TransformComponent.h"
#include "game/component/ColliderComponent.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IStringConverter.h"
#include "core/interface/ILogger.h"
#include <algorithm>
#include <array>
#include <string>

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

	// 通知バーの寸法（画面高さ比。解像度非依存にするため）。背景を画像化する前提で固定サイズにする
	constexpr float BANNER_HEIGHT_RATIO{ 0.052f };
	constexpr float BANNER_WIDTH_RATIO{ 0.18f };

	// バナー内のレイアウト比率（*_W_RATIO はバナー横幅比、*_H_RATIO はバナー高さ比）
	constexpr float PAD_X_RATIO{ 0.05f };          // 左右パディング
	constexpr float PAD_Y_RATIO{ 0.10f };          // 上パディング
	constexpr float META_FONT_H_RATIO{ 0.26f };    // アプリ名・時刻のフォントサイズ
	constexpr float SMALL_ICON_H_RATIO{ 0.32f };   // ヘッダーの小アイコンの一辺
	constexpr float NAME_GAP_W_RATIO{ 0.03f };     // アイコンと敵名の間隔
	constexpr float TIME_GAP_W_RATIO{ 0.06f };     // 敵名と時刻の間隔
	constexpr float MSG_FONT_H_RATIO{ 0.44f };     // 本文メッセージのフォントサイズ
	constexpr float MSG_TOP_GAP_H_RATIO{ 0.01f };  // ヘッダーと本文の間隔
	constexpr float FALLBACK_CORNER_H_RATIO{ 0.20f }; // 背景代替（角丸矩形）の角半径
	// 頭上へバナー下端を浮かせるワールド単位のマージン
	constexpr float HEAD_MARGIN{ 45.0f };
	// コライダーが無い敵のフォールバック頭上高さ（ワールド単位）
	constexpr float FALLBACK_HEAD_HEIGHT{ 180.0f };

	// 危険メッセージ（発見時にランダムで1つ選ぶ）
	constexpr std::array<const char*, 3> ALERT_MESSAGES{ "危険！", "見つけた！", "発見！" };

	// 左のアプリアイコン画像のリソースID
	constexpr const char* ALERT_ICON_IMAGE_ID{ "alert-icon" };
	// 通知バーの背景画像のリソースID
	constexpr const char* ALERT_BAR_IMAGE_ID{ "alert-bar" };

	// ヘッダー右端に出す時刻表記（発見直後なので「たった今」）
	constexpr const char* ALERT_TIME_TEXT{ "たった今" };

	/**
	 * @brief 発見した敵の種類名（＝擬人化アプリ名）をAIマーカーコンポーネントから求める
	 * @param componentManager ComponentManagerの参照
	 * @param entityId 対象のEntityId
	 * @return 敵の種類名（"Xcode" / "Safari" / "Mac"）
	 */
	const char* enemyTypeName(core::ecs::ComponentManager& componentManager, core::ecs::EntityId entityId)
	{
		using game::component::EnemyTypeComponent;
		const auto* type{ componentManager.tryGet<EnemyTypeComponent>(entityId) };
		return type != nullptr ? game::constant::toEnemyTypeName(type->m_type) : "Unknown";
	}
} // namespace

namespace game::system
{
	DetectionAlertVisualsSystem::DetectionAlertVisualsSystem(core::ecs::ComponentManager& componentManager,
	    core::base::EventBus& eventBus,
	    core::iface::IRenderer& renderer,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::iface::IResourceManager& resourceManager)
	    : m_componentManager{ componentManager }
	    , m_eventBus{ eventBus }
	    , m_renderer{ renderer }
	    , m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	{
		// DxLibはShift-JISで描画するため、日本語メッセージの変換器を取得しておく
		m_stringConverter = core::base::ServiceLocator::get<core::iface::IStringConverter>();

		// 左のアプリアイコン画像を読み込む。失敗時はアイコンを描かず、ここでエラーを記録する
		m_iconHandle = resourceManager.loadImageById(ALERT_ICON_IMAGE_ID);
		if (m_iconHandle == -1)
			LOG_E("発見演出のアイコン画像 '{}' の読み込みに失敗しました", ALERT_ICON_IMAGE_ID);

		// 通知バーの背景画像を読み込む。失敗時は角丸矩形で代替する
		m_barHandle = resourceManager.loadImageById(ALERT_BAR_IMAGE_ID);
		if (m_barHandle == -1)
			LOG_E("発見演出の背景バー画像 '{}' の読み込みに失敗しました", ALERT_BAR_IMAGE_ID);

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
			m_componentManager.add<component::AlertComponent>(e.m_entityId, { ALERT_DURATION, messageIndex });
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
			drawBanner(centerX, topY, enemyTypeName(m_componentManager, entityId),
			    ALERT_MESSAGES[alert.m_messageIndex]);
			m_uiRenderer.resetBlendMode();
		}
	}

	void DetectionAlertVisualsSystem::drawBanner(int centerX, int topY, const char* enemyName, const char* messageUtf8)
	{
		const float screenH{ static_cast<float>(m_screen.getHeight()) };
		const int bannerH{ static_cast<int>(screenH * BANNER_HEIGHT_RATIO) };
		const int bannerW{ static_cast<int>(screenH * BANNER_WIDTH_RATIO) };
		// 以降の std::max の第1引数（4・2 など）は最小ピクセル数。
		// 極端に小さい解像度でも余白やフォントが0pxに潰れないための下限値
		const int padX{ std::max(4, static_cast<int>(bannerW * PAD_X_RATIO)) };
		const int padY{ std::max(2, static_cast<int>(bannerH * PAD_Y_RATIO)) };

		const int x{ centerX - bannerW / 2 };
		const int y{ topY };

		using core::utility::Color;

		// 背景バー。画像があれば使い、無ければ角丸矩形で代替する
		if (m_barHandle != -1)
			m_uiRenderer.drawImage(m_barHandle, x, y, bannerW, bannerH);
		else
		{
			const int radius{ std::max(2, static_cast<int>(bannerH * FALLBACK_CORNER_H_RATIO)) };
			drawRoundedRect(x, y, bannerW, bannerH, radius, Color::ALERT_BANNER_BG);
		}

		// --- ヘッダー行：小さいアイコン＋敵の種類（アプリ名の位置） ---
		const int metaFont{ std::max(8, static_cast<int>(bannerH * META_FONT_H_RATIO)) };
		const int smallIcon{ std::max(8, static_cast<int>(bannerH * SMALL_ICON_H_RATIO)) };
		const int headerY{ y + padY };
		const int metaTextY{ headerY + (smallIcon - metaFont) / 2 };

		m_uiRenderer.drawImage(m_iconHandle, x + padX, headerY, smallIcon, smallIcon);

		// 敵の種類名（"Xcode" 等はASCIIなので変換不要）
		const int nameX{ x + padX + smallIcon + std::max(3, static_cast<int>(bannerW * NAME_GAP_W_RATIO)) };
		m_uiRenderer.drawText(nameX, metaTextY, enemyName, Color::ALERT_SUBTEXT_GRAY, metaFont);

		// 時刻（アプリ名のすぐ右に置き、名前との間の余白を詰める）
		std::string timeText{ ALERT_TIME_TEXT };
		if (m_stringConverter)
			timeText = m_stringConverter->utf8ToShiftJis(timeText);
		const int nameWidth{ m_uiRenderer.getTextWidth(enemyName, metaFont) };
		const int timeX{ nameX + nameWidth + std::max(4, static_cast<int>(bannerW * TIME_GAP_W_RATIO)) };
		m_uiRenderer.drawText(timeX, metaTextY, timeText.c_str(), Color::ALERT_SUBTEXT_GRAY, metaFont);

		// --- 本文：危険メッセージ（赤）。ヘッダーとの間の余白を詰めてぎゅっとさせる ---
		const int msgFont{ std::max(10, static_cast<int>(bannerH * MSG_FONT_H_RATIO)) };
		const int msgY{ headerY + smallIcon + std::max(1, static_cast<int>(bannerH * MSG_TOP_GAP_H_RATIO)) };
		std::string message{ messageUtf8 };
		if (m_stringConverter)
			message = m_stringConverter->utf8ToShiftJis(message);
		m_uiRenderer.drawText(x + padX, msgY, message.c_str(), Color::ALERT_DANGER_RED, msgFont);
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
