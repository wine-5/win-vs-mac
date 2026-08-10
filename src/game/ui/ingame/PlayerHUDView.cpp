#include "PlayerHUDView.h"
#include "LowHealthPulse.h"
#include "core/constant/UI.h"
#include "core/utility/Color.h"
#include "core/utility/Log.h"
#include "core/utility/MathConstants.h"
#include "game/component/combat/HealthComponent.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/combat/PlayerStatsComponent.h"
#include "game/component/combat/PlayerStatBaseComponent.h"
#include "game/component/movement/InputComponent.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// パネルの位置とサイズ（左下・1080p基準）
	constexpr int PANEL_MARGIN{ 28 };
	constexpr int PANEL_WIDTH{ 430 };
	constexpr int PANEL_HEIGHT{ 140 };
	constexpr int PANEL_PADDING{ 20 };

	// パネル内の各要素の位置（パネル左上からの相対座標・1080p基準）
	constexpr int LABEL_Y{ 16 };
	constexpr int LABEL_FONT_SIZE{ 19 };
	constexpr int VALUE_FONT_SIZE{ 18 };
	constexpr int BAR_Y{ 48 };
	constexpr int BAR_HEIGHT{ 18 };

	// 能力値の並び（1行4項目・1080p基準）
	constexpr int STAT_ROW_Y{ 76 };
	constexpr int STAT_ROW_HEIGHT{ 52 };
	constexpr int STAT_ICON_SIZE{ 42 };
	constexpr int STAT_VALUE_GAP{ 8 }; // アイコンと数値の間隔
	constexpr int STAT_FONT_SIZE{ 19 };
	constexpr int STATS_PER_PAGE{ 4 };

	// ページ送り。切り替えは上下のスライドで見せる（横に流すとパネルからはみ出すため）
	constexpr float PAGE_INTERVAL{ 5.0f };        // 自動で切り替わる間隔（秒）
	constexpr float SLIDE_DURATION{ 0.30f };      // 切り替えアニメの長さ（秒）
	constexpr int SLIDE_OFFSET{ 12 };             // スライドの振れ幅（1080p基準）
	constexpr float CHANGE_HOLD_DURATION{ 3.0f }; // 値が変わった項目を留めて強調する長さ（秒）
	constexpr float EXPAND_SPEED{ 6.0f };         // Tabで開閉する速さ（1.0を割る秒数の逆数）

	// 変化量のポップアップ。色が変わるだけでは「いくつ増えたか」が分からないため、
	// 差分そのものを浮かび上がらせて消す
	constexpr int DELTA_POPUP_FONT_SIZE{ 17 };
	constexpr int DELTA_POPUP_RISE{ 18 };        // 浮き上がる距離（1080p基準）
	constexpr int DELTA_POPUP_GAP{ 4 };          // 数値との間隔（1080p基準）
	constexpr float DELTA_POPUP_EPSILON{ 0.5f }; // これ未満の変化は出さない（整数表示で0になるため）
	// 上昇を示す緑。強化中を示す黄色（STAT_BOOSTED_COLOR）と同じ色にすると、
	// 「元から強化されている」のか「今上がった」のかが見分けられない。
	// 能力が下がる経路は無いので、下降用の色は用意しない
	constexpr unsigned int DELTA_UP_COLOR{ core::utility::Color::HUD_BUFF_GREEN };

	// 能力が変わった直後にパネルの縁を光らせる。強調の保持時間より早く消して、
	// 「今起きた」ことだけを伝える（ずっと光っていると異常の合図に見える）
	constexpr int PANEL_GLOW_ALPHA{ 160 };
	constexpr float PANEL_GLOW_FADE{ 3.0f }; // 保持時間の1/3で消えきる速さ
	constexpr int PANEL_GLOW_RADIUS{ 8 };
	constexpr int PANEL_GLOW_THICKNESS{ 2 };

	// 取得の瞬間に枠が外へ膨らんで戻る。光るだけでは「表示が変わった」に留まり、
	// 「手に入れた」という手応えが出ない。枠が動くと反応として伝わる
	constexpr float PANEL_POP_DURATION{ 0.34f };
	constexpr int PANEL_POP_INFLATE{ 10 };  // 外へ広がる量（1080p基準）
	constexpr int PANEL_POP_THICKNESS{ 3 }; // 膨らんだ枠の太さ（1080p基準）
	constexpr int PANEL_POP_ALPHA{ 255 };

	// 能力値の並び。セレクト画面（パラメータウィンドウ）と同じ8項目・同じアイコン・同じ順序で使う。
	// 順序が違うと「セレクトで見たあの位置の値」を探し直すことになるため、必ず揃える。
	// 前半4つがページ0、後半4つがページ1になる
	constexpr std::array<const char*, 8> STAT_ICON_IMAGE_IDS{
		"stat-hp", "stat-atk", "stat-def", "stat-spd",
		"stat-rng", "stat-crit", "stat-bspd", "stat-brng"
	};

	// STAT_ICON_IMAGE_IDS 上の位置。値を詰める側と並びがずれないよう名前で参照する
	constexpr int STAT_INDEX_HP{ 0 };
	constexpr int STAT_INDEX_ATK{ 1 };
	constexpr int STAT_INDEX_DEF{ 2 };
	constexpr int STAT_INDEX_SPD{ 3 };
	constexpr int STAT_INDEX_RNG{ 4 };
	// 会心率だけは割合なので百分率で見せる。この位置だけ書式が変わる
	constexpr int STAT_INDEX_CRIT{ 5 };
	constexpr int STAT_INDEX_BSPD{ 6 };
	constexpr int STAT_INDEX_BRNG{ 7 };

	// 素の値より上がっている項目の色。装備ファイル・Itemなど強化の出どころは問わない
	constexpr unsigned int STAT_BOOSTED_COLOR{ core::utility::Color::HUD_CHARGE_MAX };
	// 強化とみなす下限。浮動小数の誤差で素の値と同じものが光らないようにする
	constexpr float STAT_BOOST_EPSILON{ 0.001f };

	// 能力値を通常の濃さで描くときの不透明度（スライド中はここから落とす）
	constexpr int STAT_ALPHA_OPAQUE{ 255 };

	constexpr int BAR_GROOVE_ALPHA{ 20 }; // バーの溝（白をごく薄く敷く）

	// HP残量に応じたバーの色。Windows 11のプログレスバーに倣い単色で塗る
	constexpr unsigned int BAR_COLOR_HIGH{ 0xFF36D07B };
	constexpr unsigned int BAR_COLOR_MID{ core::utility::Color::HUD_CHARGE_MAX };
	constexpr unsigned int BAR_COLOR_LOW{ core::utility::Color::HUD_CRIT_RED };
	constexpr float BAR_MID_THRESHOLD{ 0.5f };

	// 被弾演出
	constexpr unsigned int BAR_RESIDUAL_COLOR{ core::utility::Color::HUD_CRIT_RED }; // 削られた分を示す残像
	constexpr float DAMAGE_FLASH_DURATION{ 0.20f };          // 白フラッシュの長さ（秒）
	constexpr int DAMAGE_FLASH_ALPHA{ 190 };                 // 白フラッシュの強さ
	constexpr float RESIDUAL_HOLD_DURATION{ 0.35f };         // 残像が縮み始めるまでの待ち（秒）
	constexpr float RESIDUAL_DECAY_PER_SECOND{ 0.55f };      // 残像が縮む速さ（残量比／秒）

	// 低HPの警告脈動（点滅のリズムは LowHealthPulse.h と共有する）
	constexpr unsigned int LOW_PULSE_COLOR{ core::utility::Color::HUD_CRIT_RED };
	constexpr int LOW_PULSE_ALPHA{ 130 }; // 脈動の最も明るいときの強さ

	constexpr const char* STATUS_LABEL{ "PLAYER STATUS" };
} // namespace

namespace game::ui::ingame
{
	PlayerHUDView::PlayerHUDView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::ecs::ComponentManager& componentManager,
	    core::iface::IResourceManager& resourceManager)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_componentManager{ componentManager }
	    , m_panel{ uiRenderer, screen }
	{
		// アイコンは毎フレーム引き直さず、生成時に一度だけ読み込む。
		// 失敗しても数値だけは出せるので、記録に留めて描画は続ける
		for (int i{ 0 }; i < STAT_COUNT; ++i)
		{
			m_iconHandles[i] = resourceManager.loadImageById(STAT_ICON_IMAGE_IDS[i]);
			if (m_iconHandles[i] == -1)
				core::log::error("ステータスアイコン '{}' の読み込みに失敗しました", STAT_ICON_IMAGE_IDS[i]);
		}
	}

	int PlayerHUDView::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	float PlayerHUDView::tickDeltaTime()
	{
		const auto now{ std::chrono::steady_clock::now() };
		const float deltaTime{ m_hasLastFrameTime
			                       ? std::chrono::duration<float>(now - m_lastFrameTime).count()
			                       : 0.0f };
		m_lastFrameTime = now;
		m_hasLastFrameTime = true;
		return deltaTime;
	}

	void PlayerHUDView::updateDamageReaction(float ratio, float deltaTime)
	{
		const auto now{ std::chrono::steady_clock::now() };

		// 初回は残像を実HPに合わせるだけ。ここで演出を出すと開始直後に赤帯が走ってしまう
		if (m_displayedRatio < 0.0f)
		{
			m_displayedRatio = ratio;
			m_lastRatio = ratio;
			return;
		}

		// 被弾は「実HPが前フレームより減ったか」だけで判定する。
		// 残像との差で判定すると、残像が追いつくまでの間ずっと被弾扱いになり、
		// フラッシュが何度も再点火してバーがチカチカしてしまう
		if (ratio < m_lastRatio)
		{
			m_lastDamageTime = now;
			m_isDamageFlashing = true;
		}
		m_lastRatio = ratio;

		// 回復したときは残像を追い越させる（残像は「削られた分」専用の表現なので保持しない）
		if (ratio >= m_displayedRatio)
		{
			m_displayedRatio = ratio;
			return;
		}

		// 削られた直後は残像を止めて「どれだけ減ったか」を見せ、少し置いてから追いつかせる
		if (std::chrono::duration<float>(now - m_lastDamageTime).count() < RESIDUAL_HOLD_DURATION)
			return;

		m_displayedRatio = std::max(ratio, m_displayedRatio - RESIDUAL_DECAY_PER_SECOND * deltaTime);
	}

	void PlayerHUDView::drawLowHealthPulse(int x, int y, int width, int height, int radius, float ratio)
	{
		if (!low_health::isLow(ratio))
			return;

		const float elapsed{ std::chrono::duration<float>(
			std::chrono::steady_clock::now() - m_startTime)
			    .count() };
		const int alpha{ static_cast<int>(LOW_PULSE_ALPHA * low_health::computeWave(ratio, elapsed)) };
		if (alpha <= 0)
			return;

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ADD, alpha);
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, LOW_PULSE_COLOR, true, 1);
		m_uiRenderer.resetBlendMode();
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

	std::array<float, PlayerHUDView::STAT_COUNT> PlayerHUDView::collectStats(core::ecs::EntityId playerId) const
	{
		std::array<float, STAT_COUNT> stats{};

		if (const auto* attack{ m_componentManager.tryGet<component::combat::AttackComponent>(playerId) })
		{
			stats[STAT_INDEX_ATK] = attack->m_attackPower;
			stats[STAT_INDEX_RNG] = attack->m_attackRange;
			stats[STAT_INDEX_CRIT] = attack->m_criticalRate * core::utility::RATIO_TO_PERCENT; // 割合を百分率へ
		}
		if (const auto* health{ m_componentManager.tryGet<component::combat::HealthComponent>(playerId) })
		{
			stats[STAT_INDEX_HP] = health->m_maxHp;
			stats[STAT_INDEX_DEF] = health->m_defence;
		}
		if (const auto* player{ m_componentManager.tryGet<component::combat::PlayerStatsComponent>(playerId) })
		{
			stats[STAT_INDEX_SPD] = player->m_moveSpeed;
			stats[STAT_INDEX_BSPD] = player->m_projectileSpeed;
			stats[STAT_INDEX_BRNG] = player->m_projectileRange;
		}
		return stats;
	}

	std::array<float, PlayerHUDView::STAT_COUNT> PlayerHUDView::collectBaseStats(core::ecs::EntityId playerId) const
	{
		std::array<float, STAT_COUNT> stats{};

		const auto* base{ m_componentManager.tryGet<component::combat::PlayerStatBaseComponent>(playerId) };
		if (base == nullptr)
			return stats; // 控えが無ければ強化なし扱い（全項目が素の色になる）

		stats[STAT_INDEX_HP] = base->m_maxHp;
		stats[STAT_INDEX_ATK] = base->m_attackPower;
		stats[STAT_INDEX_DEF] = base->m_defence;
		stats[STAT_INDEX_SPD] = base->m_moveSpeed;
		stats[STAT_INDEX_RNG] = base->m_attackRange;
		stats[STAT_INDEX_CRIT] = base->m_criticalRate * core::utility::RATIO_TO_PERCENT; // 現在値と同じ百分率へ揃える
		stats[STAT_INDEX_BSPD] = base->m_projectileSpeed;
		stats[STAT_INDEX_BRNG] = base->m_projectileRange;
		return stats;
	}

	void PlayerHUDView::updatePaging(const std::array<float, STAT_COUNT>& stats, bool isExpanded, float deltaTime)
	{
		// Tabの開閉は往復とも同じ速さで進める
		const float expandTarget{ isExpanded ? 1.0f : 0.0f };
		const float expandStep{ EXPAND_SPEED * deltaTime };
		if (m_expandProgress < expandTarget)
			m_expandProgress = std::min(expandTarget, m_expandProgress + expandStep);
		else
			m_expandProgress = std::max(expandTarget, m_expandProgress - expandStep);

		// 値が変わった項目を探す。初回は比較対象が無いので基準を取るだけ
		if (!m_hasPreviousStats)
		{
			m_previousStats = stats;
			m_hasPreviousStats = true;
		}
		else
		{
			for (int i{ 0 }; i < STAT_COUNT; ++i)
			{
				if (stats[i] == m_previousStats[i])
					continue;

				m_changedIndex = i;
				m_changedDelta = stats[i] - m_previousStats[i];
				m_changeHighlight = CHANGE_HOLD_DURATION;
				m_panelPop = PANEL_POP_DURATION;

				// 変わった項目が裏のページなら即座にそちらへ送る（Item取得を見逃さないため）
				const int page{ i / STATS_PER_PAGE };
				if (page != m_page)
				{
					m_slideFromPage = m_page;
					m_page = page;
					m_slideProgress = 0.0f;
				}
				m_pageTimer = CHANGE_HOLD_DURATION;
			}
			m_previousStats = stats;
		}

		if (m_changeHighlight > 0.0f)
			m_changeHighlight -= deltaTime;

		if (m_panelPop > 0.0f)
			m_panelPop = std::max(0.0f, m_panelPop - deltaTime);

		// スライド中の進行
		if (m_slideProgress < 1.0f)
			m_slideProgress = std::min(1.0f, m_slideProgress + deltaTime / SLIDE_DURATION);

		// 全項目を開いている間はページ送りを止める。読んでいる最中に動くと目が滑る
		if (isExpanded)
			return;

		m_pageTimer -= deltaTime;
		if (m_pageTimer > 0.0f)
			return;

		m_pageTimer = PAGE_INTERVAL;
		m_slideFromPage = m_page;
		m_page = (m_page + 1) % 2;
		m_slideProgress = 0.0f;
	}

	void PlayerHUDView::drawStatCell(int x, int y, int index, float value, bool isBoosted, int alpha)
	{
		const int iconSize{ scaled(STAT_ICON_SIZE) };
		const int rowHeight{ scaled(STAT_ROW_HEIGHT) };
		const int iconY{ y + (rowHeight - iconSize) / 2 };

		if (m_iconHandles[index] != -1)
		{
			m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);
			m_uiRenderer.drawImage(m_iconHandles[index], x, iconY, iconSize, iconSize);
			m_uiRenderer.resetBlendMode();
		}

		char text[16]{};
		if (index == STAT_INDEX_CRIT)
			std::snprintf(text, sizeof(text), "%d%%", static_cast<int>(value));
		else
			std::snprintf(text, sizeof(text), "%d", static_cast<int>(value));

		// 素の値より上がっている項目は黄色にして、強化されていることを一目で分かるようにする。
		// 直近で変化した項目は、そのうえで一時的に強い色にして「今上がった」ことも伝える
		const bool isChanged{ index == m_changedIndex && m_changeHighlight > 0.0f };
		unsigned int color{ core::utility::Color::HUD_INK };
		if (isChanged)
			color = DELTA_UP_COLOR;
		else if (isBoosted)
			color = STAT_BOOSTED_COLOR;

		const int fontSize{ scaled(STAT_FONT_SIZE) };
		const int textY{ y + (rowHeight - fontSize) / 2 };

		m_uiRenderer.setFont(core::constant::ui::MONO_FONT_NAME);
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);
		m_uiRenderer.drawText(x + iconSize + scaled(STAT_VALUE_GAP), textY, text, color, fontSize);
		m_uiRenderer.resetBlendMode();

		// 変化した項目には増減量を浮かび上がらせる。数値が変わったことは色でも分かるが、
		// 「いくつ増えたか」は元の値を覚えていないと分からないため、差分そのものを見せる
		if (isChanged && m_changedDelta >= DELTA_POPUP_EPSILON)
		{
			const float progress{ 1.0f - m_changeHighlight / CHANGE_HOLD_DURATION };
			const int rise{ static_cast<int>(scaled(DELTA_POPUP_RISE) * progress) };
			const int popupAlpha{ static_cast<int>(alpha * std::clamp(1.0f - progress, 0.0f, 1.0f)) };

			char deltaText[16]{};
			std::snprintf(deltaText, sizeof(deltaText), "+%d", static_cast<int>(m_changedDelta));

			m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, popupAlpha);
			m_uiRenderer.drawText(x + iconSize + scaled(STAT_VALUE_GAP), textY - rise - scaled(DELTA_POPUP_GAP),
			    deltaText, DELTA_UP_COLOR, scaled(DELTA_POPUP_FONT_SIZE));
			m_uiRenderer.resetBlendMode();
		}

		m_uiRenderer.resetFont();
	}

	void PlayerHUDView::drawStatChangeReaction(int x, int y, int width, int height)
	{
		if (m_changeHighlight <= 0.0f)
			return;

		// 枠が外へ膨らんで戻る。中身は動かさず枠だけを動かすので、
		// 数値を読んでいる最中でもレイアウトが揺れない
		if (m_panelPop > 0.0f)
		{
			const float popProgress{ 1.0f - m_panelPop / PANEL_POP_DURATION };

			// 一気に開いてゆっくり戻る。等速で往復すると呼吸のように見えて反応に見えない
			const float swell{ std::sin(popProgress * core::utility::PI) };
			const int inflate{ static_cast<int>(scaled(PANEL_POP_INFLATE) * swell) };
			const int popAlpha{ static_cast<int>(PANEL_POP_ALPHA * swell) };

			if (popAlpha > 0)
			{
				m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ADD, popAlpha);
				m_uiRenderer.drawRoundedBox(x - inflate, y - inflate,
				    width + inflate * 2, height + inflate * 2,
				    scaled(PANEL_GLOW_RADIUS) + inflate, DELTA_UP_COLOR, false,
				    scaled(PANEL_POP_THICKNESS));
				m_uiRenderer.resetBlendMode();
			}
		}

		// 膨らみが収まった後も、しばらく縁を光らせて「今変わった」状態を保つ
		const float progress{ 1.0f - m_changeHighlight / CHANGE_HOLD_DURATION };
		const int glowAlpha{ static_cast<int>(
			PANEL_GLOW_ALPHA * std::clamp(1.0f - progress * PANEL_GLOW_FADE, 0.0f, 1.0f)) };
		if (glowAlpha <= 0)
			return;

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ADD, glowAlpha);
		m_uiRenderer.drawRoundedBox(x, y, width, height,
		    scaled(PANEL_GLOW_RADIUS), DELTA_UP_COLOR, false, scaled(PANEL_GLOW_THICKNESS));
		m_uiRenderer.resetBlendMode();
	}

	void PlayerHUDView::drawStatPage(int x, int y, int cellWidth,
	    const std::array<float, STAT_COUNT>& stats,
	    const std::array<float, STAT_COUNT>& baseStats, int page, int alpha)
	{
		if (alpha <= 0)
			return;

		const int first{ page * STATS_PER_PAGE };
		for (int i{ 0 }; i < STATS_PER_PAGE; ++i)
		{
			const int index{ first + i };
			// 素の値を「上回っているか」だけを見る。強化がどこから来たか（装備・Item）は問わない
			const bool isBoosted{ stats[index] > baseStats[index] + STAT_BOOST_EPSILON };
			drawStatCell(x + cellWidth * i, y, index, stats[index], isBoosted, alpha);
		}
	}

	void PlayerHUDView::draw(core::ecs::EntityId playerId)
	{
		if (!m_componentManager.has<component::combat::HealthComponent>(playerId))
			return;

		const auto& health{ m_componentManager.get<component::combat::HealthComponent>(playerId) };
		if (health.m_maxHp <= 0.0f)
			return;

		const float deltaTime{ tickDeltaTime() };

		// Tabを押している間だけ8項目すべてを開く（押せる見た目のUIを置かずに全項目へ到達させる）
		const auto* input{ m_componentManager.tryGet<component::movement::InputComponent>(playerId) };
		const bool isExpanded{ input != nullptr && input->m_statusViewPressed };

		const auto stats{ collectStats(playerId) };
		const auto baseStats{ collectBaseStats(playerId) };
		updatePaging(stats, isExpanded, deltaTime);

		const int panelWidth{ scaled(PANEL_WIDTH) };
		const int rowHeight{ scaled(STAT_ROW_HEIGHT) };
		// 開いている間は2行ぶんへ伸ばす。高さを連続に変えることで、開閉が引き出しのように見える
		const int panelHeight{ scaled(PANEL_HEIGHT) + static_cast<int>(rowHeight * m_expandProgress) };
		const int panelX{ scaled(PANEL_MARGIN) };
		const int panelY{ m_screen.getHeight() - scaled(PANEL_MARGIN) - panelHeight };

		m_panel.draw(panelX, panelY, panelWidth, panelHeight);
		drawStatChangeReaction(panelX, panelY, panelWidth, panelHeight);

		// 左に見出し、右にHPの実数値。数値は桁が動いても右端が揃うよう右寄せで置く
		const int padding{ scaled(PANEL_PADDING) };
		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
		m_uiRenderer.drawText(panelX + padding, panelY + scaled(LABEL_Y), STATUS_LABEL,
		    core::utility::Color::HUD_INK, scaled(LABEL_FONT_SIZE));

		char hpText[32]{};
		std::snprintf(hpText, sizeof(hpText), "HP %d / %d",
		    static_cast<int>(health.m_currentHp), static_cast<int>(health.m_maxHp));

		m_uiRenderer.setFont(core::constant::ui::MONO_FONT_NAME);
		const int valueFontSize{ scaled(VALUE_FONT_SIZE) };
		const int valueWidth{ m_uiRenderer.getTextWidth(hpText, valueFontSize) };
		m_uiRenderer.drawText(panelX + panelWidth - padding - valueWidth, panelY + scaled(LABEL_Y),
		    hpText, core::utility::Color::HUD_INK, valueFontSize);
		m_uiRenderer.resetFont();

		// 0除算はmaxHpのチェックで防いでいる。回復過多などで1.0を超えても溝からはみ出さないよう丸める
		const float ratio{ std::clamp(health.m_currentHp / health.m_maxHp, 0.0f, 1.0f) };
		updateDamageReaction(ratio, deltaTime);
		drawHealthBar(panelX + padding, panelY + scaled(BAR_Y),
		    panelWidth - padding * 2, scaled(BAR_HEIGHT), ratio);

		drawStats(panelX + padding, panelY + scaled(STAT_ROW_Y),
		    (panelWidth - padding * 2) / STATS_PER_PAGE, stats, baseStats);
	}

	void PlayerHUDView::drawStats(int x, int y, int cellWidth,
	    const std::array<float, STAT_COUNT>& stats,
	    const std::array<float, STAT_COUNT>& baseStats)
	{
		// 開ききっている間は2ページを縦に並べて全項目を見せる（この間はページ送りが止まっている）
		if (m_expandProgress >= 1.0f)
		{
			drawStatPage(x, y, cellWidth, stats, baseStats, 0, STAT_ALPHA_OPAQUE);
			drawStatPage(x, y + scaled(STAT_ROW_HEIGHT), cellWidth, stats, baseStats, 1,
			    STAT_ALPHA_OPAQUE);
			return;
		}

		// 開閉の途中：2ページ目は伸びる高さに合わせて濃さも上げる
		if (m_expandProgress > 0.0f)
		{
			drawStatPage(x, y, cellWidth, stats, baseStats, 0, STAT_ALPHA_OPAQUE);
			drawStatPage(x, y + scaled(STAT_ROW_HEIGHT), cellWidth, stats, baseStats, 1,
			    static_cast<int>(STAT_ALPHA_OPAQUE * m_expandProgress));
			return;
		}

		// 平常時：切り替え中は前のページが上へ抜け、次のページが下から入る。
		// ドット等の指標を置かなくても、この動き自体が「続きがある」ことを伝える
		if (m_slideProgress >= 1.0f)
		{
			drawStatPage(x, y, cellWidth, stats, baseStats, m_page, STAT_ALPHA_OPAQUE);
			return;
		}

		const int offset{ scaled(SLIDE_OFFSET) };
		const float t{ m_slideProgress };
		drawStatPage(x, y - static_cast<int>(offset * t), cellWidth, stats, baseStats, m_slideFromPage,
		    static_cast<int>(STAT_ALPHA_OPAQUE * (1.0f - t)));
		drawStatPage(x, y + static_cast<int>(offset * (1.0f - t)), cellWidth, stats, baseStats, m_page,
		    static_cast<int>(STAT_ALPHA_OPAQUE * t));
	}

	void PlayerHUDView::drawHealthBar(int x, int y, int width, int height, float ratio)
	{
		// 高さの半分を半径にすると、Windows 11のプログレスバーと同じピル形状になる
		const int radius{ height / 2 };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, BAR_GROOVE_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, core::utility::Color::WHITE, true, 1);
		m_uiRenderer.resetBlendMode();

		// ピル形状は幅が高さを下回ると成立しないため、残量が僅かでも高さぶんの幅は確保する。
		// 打ち切ってしまうと、残りHPが数％のときにバーが空になって「もう死んでいる」ように見える
		auto pillWidth = [&](float value)
		{ return std::max(height, static_cast<int>(width * value)); };

		// 実HPより先に、遅れて縮む残像を赤で描く。実HPのバーが上に重なるので、
		// はみ出した赤い帯＝直前に失った分として読める（格闘ゲームの体力バーと同じ仕組み）
		if (m_displayedRatio > 0.0f)
			m_uiRenderer.drawRoundedBox(x, y, pillWidth(m_displayedRatio), height, radius,
			    BAR_RESIDUAL_COLOR, true, 1);

		// 倒れている間はバーを空にする（ここだけは何も描かないのが正しい状態）
		if (ratio <= 0.0f)
			return;

		const int fillWidth{ pillWidth(ratio) };

		unsigned int fillColor{ BAR_COLOR_HIGH };
		if (low_health::isLow(ratio))
			fillColor = BAR_COLOR_LOW;
		else if (ratio <= BAR_MID_THRESHOLD)
			fillColor = BAR_COLOR_MID;

		m_uiRenderer.drawRoundedBox(x, y, fillWidth, height, radius, fillColor, true, 1);

		drawLowHealthPulse(x, y, fillWidth, height, radius, ratio);

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
