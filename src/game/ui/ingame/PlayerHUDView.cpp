#include "PlayerHUDView.h"
#include "core/constant/UI.h"
#include "core/utility/Color.h"
#include "game/component/combat/HealthComponent.h"
#include <algorithm>
#include <cstdio>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// パネルの位置とサイズ（左下・1080p基準）
	constexpr int PANEL_MARGIN{ 28 };
	constexpr int PANEL_WIDTH{ 380 };
	constexpr int PANEL_HEIGHT{ 92 };
	constexpr int PANEL_PADDING{ 20 };

	// パネル内の各要素の位置（パネル左上からの相対座標・1080p基準）
	constexpr int LABEL_Y{ 16 };
	constexpr int LABEL_FONT_SIZE{ 19 };
	constexpr int VALUE_FONT_SIZE{ 18 };
	constexpr int BAR_Y{ 54 };
	constexpr int BAR_HEIGHT{ 18 };

	constexpr int BAR_GROOVE_ALPHA{ 20 }; // バーの溝（白をごく薄く敷く）

	// HP残量に応じたバーの色。Windows 11のプログレスバーに倣い単色で塗る
	constexpr unsigned int BAR_COLOR_HIGH{ 0xFF36D07B };
	constexpr unsigned int BAR_COLOR_MID{ 0xFFFFC83D };
	constexpr unsigned int BAR_COLOR_LOW{ 0xFFE81123 };
	constexpr float BAR_MID_THRESHOLD{ 0.5f };
	constexpr float BAR_LOW_THRESHOLD{ 0.2f };

	// 被弾演出
	constexpr unsigned int BAR_RESIDUAL_COLOR{ 0xFFE81123 }; // 削られた分を示す残像
	constexpr float DAMAGE_FLASH_DURATION{ 0.20f };          // 白フラッシュの長さ（秒）
	constexpr int DAMAGE_FLASH_ALPHA{ 190 };                 // 白フラッシュの強さ
	constexpr float RESIDUAL_HOLD_DURATION{ 0.35f };         // 残像が縮み始めるまでの待ち（秒）
	constexpr float RESIDUAL_DECAY_PER_SECOND{ 0.55f };      // 残像が縮む速さ（残量比／秒）

	// フォント。数値・英字は等幅、日本語を含みうるラベルはNoto Sans JPで描く
	constexpr const char* MONO_FONT_NAME{ "Cascadia Mono" };
	constexpr const char* UI_FONT_NAME{ "Noto Sans JP" };

	constexpr const char* STATUS_LABEL{ "PLAYER STATUS" };
} // namespace

namespace game::ui::ingame
{
	PlayerHUDView::PlayerHUDView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::ecs::ComponentManager& componentManager)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_componentManager{ componentManager }
	    , m_panel{ uiRenderer, screen }
	{
	}

	int PlayerHUDView::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	void PlayerHUDView::updateDamageReaction(float ratio)
	{
		const auto now{ std::chrono::steady_clock::now() };
		const float deltaTime{ m_hasLastFrameTime
			                       ? std::chrono::duration<float>(now - m_lastFrameTime).count()
			                       : 0.0f };
		m_lastFrameTime = now;
		m_hasLastFrameTime = true;

		// 初回は残像を実HPに合わせるだけ。ここで演出を出すと開始直後に赤帯が走ってしまう
		if (m_displayedRatio < 0.0f)
		{
			m_displayedRatio = ratio;
			return;
		}

		// 回復したときは残像を追い越させる（残像は「削られた分」専用の表現なので保持しない）
		if (ratio >= m_displayedRatio)
		{
			m_displayedRatio = ratio;
			return;
		}

		// 減った瞬間を被弾とみなす。HealthComponentの値の比較だけで足りるためイベントは購読しない
		const bool isNewDamage{ !m_isDamageFlashing ||
			                    std::chrono::duration<float>(now - m_lastDamageTime).count() > DAMAGE_FLASH_DURATION };
		if (isNewDamage)
		{
			m_lastDamageTime = now;
			m_isDamageFlashing = true;
		}

		// 削られた直後は残像を止めて「どれだけ減ったか」を見せ、少し置いてから追いつかせる
		if (std::chrono::duration<float>(now - m_lastDamageTime).count() < RESIDUAL_HOLD_DURATION)
			return;

		m_displayedRatio = std::max(ratio, m_displayedRatio - RESIDUAL_DECAY_PER_SECOND * deltaTime);
	}

	float PlayerHUDView::getDamageFlashProgress() const
	{
		if (!m_isDamageFlashing)
			return 1.0f;

		const float elapsed{ std::chrono::duration<float>(
			std::chrono::steady_clock::now() - m_lastDamageTime)
			    .count() };
		return std::clamp(elapsed / DAMAGE_FLASH_DURATION, 0.0f, 1.0f);
	}

	void PlayerHUDView::draw(core::ecs::EntityId playerId)
	{
		if (!m_componentManager.has<component::combat::HealthComponent>(playerId))
			return;

		const auto& health{ m_componentManager.get<component::combat::HealthComponent>(playerId) };
		if (health.m_maxHp <= 0.0f)
			return;

		const int panelWidth{ scaled(PANEL_WIDTH) };
		const int panelHeight{ scaled(PANEL_HEIGHT) };
		const int panelX{ scaled(PANEL_MARGIN) };
		const int panelY{ m_screen.getHeight() - scaled(PANEL_MARGIN) - panelHeight };

		m_panel.draw(panelX, panelY, panelWidth, panelHeight);

		// 左に見出し、右にHPの実数値。数値は桁が動いても右端が揃うよう右寄せで置く
		const int padding{ scaled(PANEL_PADDING) };
		m_uiRenderer.setFont(UI_FONT_NAME);
		m_uiRenderer.drawText(panelX + padding, panelY + scaled(LABEL_Y), STATUS_LABEL,
		    core::utility::Color::HUD_INK, scaled(LABEL_FONT_SIZE));

		char hpText[32]{};
		std::snprintf(hpText, sizeof(hpText), "HP %d / %d",
		    static_cast<int>(health.m_currentHp), static_cast<int>(health.m_maxHp));

		m_uiRenderer.setFont(MONO_FONT_NAME);
		const int valueFontSize{ scaled(VALUE_FONT_SIZE) };
		const int valueWidth{ m_uiRenderer.getTextWidth(hpText, valueFontSize) };
		m_uiRenderer.drawText(panelX + panelWidth - padding - valueWidth, panelY + scaled(LABEL_Y),
		    hpText, core::utility::Color::HUD_INK, valueFontSize);
		m_uiRenderer.resetFont();

		// 0除算はmaxHpのチェックで防いでいる。回復過多などで1.0を超えても溝からはみ出さないよう丸める
		const float ratio{ std::clamp(health.m_currentHp / health.m_maxHp, 0.0f, 1.0f) };
		updateDamageReaction(ratio);
		drawHealthBar(panelX + padding, panelY + scaled(BAR_Y),
		    panelWidth - padding * 2, scaled(BAR_HEIGHT), ratio);
	}

	void PlayerHUDView::drawHealthBar(int x, int y, int width, int height, float ratio)
	{
		// 高さの半分を半径にすると、Windows 11のプログレスバーと同じピル形状になる
		const int radius{ height / 2 };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, BAR_GROOVE_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, core::utility::Color::WHITE, true, 1);
		m_uiRenderer.resetBlendMode();

		// 実HPより先に、遅れて縮む残像を赤で描く。実HPのバーが上に重なるので、
		// はみ出した赤い帯＝直前に失った分として読める（格闘ゲームの体力バーと同じ仕組み）
		const int residualWidth{ static_cast<int>(width * m_displayedRatio) };
		if (residualWidth >= height)
			m_uiRenderer.drawRoundedBox(x, y, residualWidth, height, radius, BAR_RESIDUAL_COLOR, true, 1);

		const int fillWidth{ static_cast<int>(width * ratio) };
		// 残りわずかなときに潰れて見えなくなるのを防ぐため、ピルが成立する最小幅を確保する
		if (fillWidth < height)
			return;

		unsigned int fillColor{ BAR_COLOR_HIGH };
		if (ratio <= BAR_LOW_THRESHOLD)
			fillColor = BAR_COLOR_LOW;
		else if (ratio <= BAR_MID_THRESHOLD)
			fillColor = BAR_COLOR_MID;

		m_uiRenderer.drawRoundedBox(x, y, fillWidth, height, radius, fillColor, true, 1);

		// 被弾直後だけバー全体を白く飛ばす。減ったことに気づく手がかりを残像より前に出す
		const float flash{ getDamageFlashProgress() };
		if (flash >= 1.0f)
			return;

		const int flashAlpha{ static_cast<int>(DAMAGE_FLASH_ALPHA * (1.0f - flash)) };
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ADD, flashAlpha);
		m_uiRenderer.drawRoundedBox(x, y, fillWidth, height, radius, core::utility::Color::WHITE, true, 1);
		m_uiRenderer.resetBlendMode();
	}
} // namespace game::ui::ingame
