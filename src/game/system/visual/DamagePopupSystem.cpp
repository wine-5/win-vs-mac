#include "DamagePopupSystem.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/TagComponent.h"
#include "game/constant/Tag.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"
#include <algorithm>
#include <string>

namespace
{
	// 数値の表示時間（秒）
	constexpr float POPUP_DURATION{ 0.9f };
	// 表示終了間際にフェードアウトを始める割合（0.6なら後半40%で薄くなる）
	constexpr float FADE_START_RATIO{ 0.55f };
	// 表示中に浮き上がる高さ（ワールド単位）
	constexpr float RISE_HEIGHT{ 70.0f };
	// 頭上へ数値を浮かせるマージン（ワールド単位）
	constexpr float HEAD_MARGIN{ 30.0f };
	// コライダーが無い相手のフォールバック頭上高さ（ワールド単位）
	constexpr float FALLBACK_HEAD_HEIGHT{ 180.0f };

	// 文字サイズ（画面高さ比。解像度に依存させないため）
	constexpr float FONT_HEIGHT_RATIO{ 0.030f };
	// クリティカル時の文字サイズ倍率。通常の数値に紛れないよう一回り大きく出す
	constexpr float CRITICAL_FONT_SCALE{ 1.6f };
	// 数値の上に添える「CRITICAL」ラベル。数値の大きさと色だけでは何が起きたか伝わらないため
	constexpr const char* CRITICAL_LABEL{ "CRITICAL" };
	// ラベルの文字サイズ（クリティカル数値に対する比）。主役は数値なので小さく置く
	constexpr float CRITICAL_LABEL_SCALE{ 0.45f };
	// ラベルと数値の縦の間隔（ラベルの文字サイズ比）
	constexpr float CRITICAL_LABEL_GAP_RATIO{ 0.25f };

	// クリティカル時に数値の背後で弾ける円。
	// 文字の大きさや色だけでは通常ダメージと見分けにくいため、「動き」で気づかせる。
	// 文字サイズを動かすとサイズごとにフォントハンドルが増えてしまうので、円で表現する
	constexpr float CRITICAL_BURST_DURATION{ 0.28f };   // 弾けきるまでの時間（秒）
	constexpr float CRITICAL_BURST_START_RATIO{ 0.3f }; // 開始半径（数値の文字サイズ比）
	constexpr float CRITICAL_BURST_END_RATIO{ 1.9f };   // 終了半径（数値の文字サイズ比）
	constexpr int CRITICAL_BURST_THICKNESS{ 3 };        // 円の線の太さ
	// 影を落とすずらし量（文字サイズ比）。明るい床でも輪郭が残るようにする
	constexpr float SHADOW_OFFSET_RATIO{ 0.09f };

	// 画面に映っているか（worldToScreenのzが0〜1の範囲内か）の判定境界
	constexpr float DEPTH_MIN{ 0.0f };
	constexpr float DEPTH_MAX{ 1.0f };

	// 半透明合成に渡す不透明度の最大値
	constexpr float ALPHA_MAX{ 255.0f };
} // namespace

namespace game::system::visual
{
	DamagePopupSystem::DamagePopupSystem(core::ecs::ComponentManager& componentManager,
	    core::base::EventBus& eventBus,
	    core::iface::IRenderer& renderer,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen)
	    : m_componentManager{ componentManager }
	    , m_renderer{ renderer }
	    , m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	{
		m_subscriptions.push_back(eventBus.subscribe<game::event::AttackHitEvent>(
		    [this](const game::event::AttackHitEvent& e)
		    { onAttackHit(e); }));
	}

	void DamagePopupSystem::onAttackHit(const game::event::AttackHitEvent& event)
	{
		// 数値を出すのは敵に与えたダメージだけ。自分の被弾は画面演出（赤ビネット等）で
		// 伝えており、そこへ数値を重ねても視界を塞ぐだけになる
		const auto* tag{ m_componentManager.tryGet<component::TagComponent>(event.m_targetId) };
		if (tag == nullptr || tag->m_tag != constant::Tag::Enemy)
			return;

		const auto* transform{ m_componentManager.tryGet<component::movement::TransformComponent>(event.m_targetId) };
		if (transform == nullptr)
			return;

		// 発生位置は相手の頭上。以後は相手に追従せずその場に留まるため、
		// 敵が移動しても数値が引っ張られない
		float headHeight{ FALLBACK_HEAD_HEIGHT };
		if (const auto* collider{ m_componentManager.tryGet<component::combat::ColliderComponent>(event.m_targetId) })
			headHeight = collider->m_size.y;

		Popup popup{};
		popup.m_worldPosition = core::Vector3{
			transform->m_position.x,
			transform->m_position.y + headHeight + HEAD_MARGIN,
			transform->m_position.z
		};
		// 小数を出しても情報にならないので整数へ丸める。0ダメージでも当たった事実は見せる
		popup.m_damage = static_cast<int>(event.m_damage + 0.5f);
		popup.m_isCritical = event.m_isCritical;
		m_popups.push_back(popup);
	}

	void DamagePopupSystem::update(float deltaTime)
	{
		for (auto& popup : m_popups)
			popup.m_elapsedTime += deltaTime;

		std::erase_if(m_popups, [](const Popup& popup)
		    { return popup.m_elapsedTime >= POPUP_DURATION; });
	}

	void DamagePopupSystem::draw()
	{
		if (m_popups.empty())
			return;

		const int normalFontSize{ static_cast<int>(m_screen.getHeight() * FONT_HEIGHT_RATIO) };
		const int criticalFontSize{ static_cast<int>(normalFontSize * CRITICAL_FONT_SCALE) };

		for (const auto& popup : m_popups)
		{
			const float progress{ popup.m_elapsedTime / POPUP_DURATION };

			// 時間とともに浮き上がる（減速しながら上がると軽く見える）
			core::Vector3 world{ popup.m_worldPosition };
			world.y += RISE_HEIGHT * (1.0f - (1.0f - progress) * (1.0f - progress));

			const core::Vector3 screen{ m_renderer.worldToScreen(world) };
			// カメラの背後や描画範囲外は出さない
			if (screen.z < DEPTH_MIN || screen.z > DEPTH_MAX)
				continue;

			// 後半だけフェードアウトする（出た瞬間ははっきり見せる）
			float alpha{ 1.0f };
			if (progress > FADE_START_RATIO)
				alpha = 1.0f - (progress - FADE_START_RATIO) / (1.0f - FADE_START_RATIO);

			// クリティカルは大きさ・色・上に添えるラベルの3点で通常の数値と区別する。
			// どれか1つだけだと、数値が飛び交う中では見落とす
			const int fontSize{ popup.m_isCritical ? criticalFontSize : normalFontSize };
			const int shadowOffset{ std::max(1, static_cast<int>(fontSize * SHADOW_OFFSET_RATIO)) };
			const unsigned int textColor{ popup.m_isCritical
				? core::utility::Color::HUD_CHARGE_MAX
				: core::utility::Color::WHITE };

			const std::string text{ std::to_string(popup.m_damage) };
			const int textWidth{ m_uiRenderer.getTextWidth(text.c_str(), fontSize) };
			const int x{ static_cast<int>(screen.x) - textWidth / 2 };
			const int y{ static_cast<int>(screen.y) };

			const int alphaParam{ static_cast<int>(ALPHA_MAX * std::clamp(alpha, 0.0f, 1.0f)) };

			// 弾ける円は数値より先に描いて背後へ回す。数値本体より短命で、独自に薄れていく
			if (popup.m_isCritical && popup.m_elapsedTime < CRITICAL_BURST_DURATION)
			{
				const float burst{ popup.m_elapsedTime / CRITICAL_BURST_DURATION };
				// 一気に広がって減速する（勢いよく弾けたように見せる）
				const float eased{ 1.0f - (1.0f - burst) * (1.0f - burst) };
				const float radiusRatio{ CRITICAL_BURST_START_RATIO + (CRITICAL_BURST_END_RATIO - CRITICAL_BURST_START_RATIO) * eased };
				const int burstAlpha{ static_cast<int>(ALPHA_MAX * (1.0f - burst)) };

				m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, burstAlpha);
				m_uiRenderer.drawCircle(static_cast<int>(screen.x),
				    static_cast<int>(screen.y) + fontSize / 2,
				    static_cast<int>(fontSize * radiusRatio),
				    core::utility::Color::HUD_CHARGE_MAX, false, CRITICAL_BURST_THICKNESS);
				m_uiRenderer.resetBlendMode();
			}

			m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alphaParam);
			// 床は真っ黒からほぼ白まであるため、影を先に置いてどちらでも輪郭が残るようにする
			m_uiRenderer.drawText(x + shadowOffset, y + shadowOffset, text.c_str(),
			    core::utility::Color::BLACK, fontSize);
			m_uiRenderer.drawText(x, y, text.c_str(), textColor, fontSize);

			// クリティカルのみ、数値の真上に「CRITICAL」を添える。
			// 数値と同じ色・同じフェードで動かし、ひとかたまりに見せる
			if (popup.m_isCritical)
			{
				const int labelFontSize{ static_cast<int>(fontSize * CRITICAL_LABEL_SCALE) };
				const int labelWidth{ m_uiRenderer.getTextWidth(CRITICAL_LABEL, labelFontSize) };
				const int labelX{ static_cast<int>(screen.x) - labelWidth / 2 };
				const int labelY{ y - labelFontSize - static_cast<int>(labelFontSize * CRITICAL_LABEL_GAP_RATIO) };
				const int labelShadow{ std::max(1, static_cast<int>(labelFontSize * SHADOW_OFFSET_RATIO)) };

				m_uiRenderer.drawText(labelX + labelShadow, labelY + labelShadow, CRITICAL_LABEL,
				    core::utility::Color::BLACK, labelFontSize);
				m_uiRenderer.drawText(labelX, labelY, CRITICAL_LABEL, textColor, labelFontSize);
			}

			m_uiRenderer.resetBlendMode();
		}
	}
} // namespace game::system::visual
