#include "BattleStartSystem.h"
#include "game/component/movement/InputComponent.h"
#include "game/component/ai/AIComponent.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"
#include "core/constant/SeType.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IAudioManager.h"
#include <algorithm>
#include <cmath>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// --- タイムライン（秒。先頭からの累積で区切る） ---
	constexpr float READY_FADE_IN{ 0.35f };  // READYが浮かび上がる
	constexpr float READY_HOLD{ 0.55f };     // READYを見せる
	constexpr float READY_FADE_OUT{ 0.30f }; // READYが消える
	constexpr float FIGHT_HOLD{ 0.45f };     // FIGHT!を強く見せる
	constexpr float FIGHT_FADE_OUT{ 0.60f }; // FIGHT!が消える

	// FIGHT!が出る瞬間＝操作解禁の時刻
	constexpr float FIGHT_TIME{ READY_FADE_IN + READY_HOLD + READY_FADE_OUT };
	constexpr float TOTAL_TIME{ FIGHT_TIME + FIGHT_HOLD + FIGHT_FADE_OUT };

	// --- 見た目（1080p基準） ---
	constexpr const char* MONO_FONT_NAME{ "Cascadia Mono SemiBold" };

	// 「Windowsの内部」という世界観に合わせ、システムメッセージのような等幅＋字間で見せる
	constexpr const char* READY_TEXT{ "R E A D Y" };
	constexpr const char* FIGHT_TEXT{ "F I G H T !" };

	constexpr int TEXT_CENTER_Y_RATIO_PERCENT{ 40 }; // 文字の中心の高さ（画面高さに対する％）
	constexpr int READY_FONT_SIZE{ 52 };
	constexpr int FIGHT_FONT_SIZE{ 104 };
	constexpr int READY_RISE{ 18 }; // READYがフェードインしながらせり上がる量
	constexpr int FIGHT_RISE{ 26 }; // FIGHT!が消えながら浮き上がる量

	// 文字の背景に敷く暗い帯（背景が明るくても文字が読めるようにする）
	constexpr int BAND_HEIGHT{ 170 };
	constexpr int BAND_ALPHA{ 130 };

	// FIGHT!の下に走る発光ライン（一瞬で左右へ開く）
	constexpr int LINE_THICKNESS{ 4 };
	constexpr int LINE_GAP{ 30 };             // 文字の下端からラインまでの間隔
	constexpr int LINE_HALF_WIDTH{ 420 };     // 開ききったときの片側の長さ
	constexpr float LINE_EXPAND_TIME{ 0.2f }; // 開ききるまでの時間（秒）

	// FIGHT!の瞬間に画面全体を白く飛ばす（打撃感を出す）
	constexpr float FLASH_TIME{ 0.14f };
	constexpr int FLASH_ALPHA{ 150 };

	/**
	 * @brief 滑らかな0→1補間（smoothstep）。等速より緩急がついて文字の出入りが上品になる
	 * @param t 進行度（0〜1）
	 * @return 補間値（0〜1）
	 */
	float smoothstep(float t)
	{
		t = std::clamp(t, 0.0f, 1.0f);
		return t * t * (3.0f - 2.0f * t);
	}
} // namespace

namespace game::system::visual
{
	BattleStartSystem::BattleStartSystem(core::ecs::ComponentManager& componentManager,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::ecs::EntityId playerId)
	    : m_componentManager{ componentManager }
	    , m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_playerId{ playerId }
	{
		// Systemの登録順に関係なく初回フレームから止めたいので、update待ちではなくここでロックする
		setGameplayLocked(true);

		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio)
			audio->playSe(core::constant::SeType::BattleReady);
	}

	bool BattleStartSystem::isPreparing() const noexcept
	{
		return m_isLocked;
	}

	int BattleStartSystem::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	void BattleStartSystem::setGameplayLocked(bool isLocked)
	{
		m_isLocked = isLocked;

		if (m_componentManager.has<component::movement::InputComponent>(m_playerId))
			m_componentManager.get<component::movement::InputComponent>(m_playerId).m_locked = isLocked;

		if (isLocked)
		{
			// 止めた敵を控えておき、解禁時にこれらだけを戻す。
			// 一律にtrueへ戻すと、演出中に死んだ敵のAIまで復活してしまう
			m_suspendedEnemyIds.clear();
			for (const auto enemyId : m_componentManager.getAllEntities<component::ai::AIComponent>())
			{
				auto& ai{ m_componentManager.get<component::ai::AIComponent>(enemyId) };
				if (!ai.m_isActive)
					continue;
				ai.m_isActive = false;
				m_suspendedEnemyIds.push_back(enemyId);
			}
			return;
		}

		for (const auto enemyId : m_suspendedEnemyIds)
		{
			if (m_componentManager.has<component::ai::AIComponent>(enemyId))
				m_componentManager.get<component::ai::AIComponent>(enemyId).m_isActive = true;
		}
		m_suspendedEnemyIds.clear();
	}

	void BattleStartSystem::update(float deltaTime)
	{
		if (!m_isPlaying)
			return;

		m_elapsedTime += deltaTime;

		// FIGHT!が出た瞬間に操作と敵AIを解禁する。文字が消えるのを待たせると、
		// もう動けるのか分からない空白の時間ができる
		if (m_isLocked && m_elapsedTime >= FIGHT_TIME)
		{
			setGameplayLocked(false);

			auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
			if (audio)
				audio->playSe(core::constant::SeType::BattleFight);
		}

		if (m_elapsedTime >= TOTAL_TIME)
			m_isPlaying = false;
	}

	void BattleStartSystem::draw()
	{
		if (!m_isPlaying)
			return;

		if (m_elapsedTime < FIGHT_TIME)
			drawReady();
		else
			drawFight();
	}

	void BattleStartSystem::drawReady()
	{
		// フェードイン→ホールド→フェードアウトの濃さ（0〜1）を求める
		float alphaRate{ 1.0f };
		if (m_elapsedTime < READY_FADE_IN)
			alphaRate = smoothstep(m_elapsedTime / READY_FADE_IN);
		else if (m_elapsedTime >= READY_FADE_IN + READY_HOLD)
			alphaRate = 1.0f - smoothstep((m_elapsedTime - READY_FADE_IN - READY_HOLD) / READY_FADE_OUT);

		if (alphaRate <= 0.0f)
			return;

		const int screenWidth{ m_screen.getWidth() };
		const int centerY{ m_screen.getHeight() * TEXT_CENTER_Y_RATIO_PERCENT / 100 };

		// 背景の暗い帯（文字と一緒に濃くなる）
		const int bandHeight{ scaled(BAND_HEIGHT) };
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA,
		    static_cast<int>(BAND_ALPHA * alphaRate));
		m_uiRenderer.drawBox(0, centerY - bandHeight / 2, screenWidth, bandHeight,
		    core::utility::Color::BLACK, true);

		// フェードインの間だけ下からせり上がらせる（消えるときは動かさない）
		const float riseRate{ 1.0f - smoothstep(m_elapsedTime / READY_FADE_IN) };
		const int fontSize{ scaled(READY_FONT_SIZE) };
		const int textWidth{ m_uiRenderer.getTextWidth(READY_TEXT, fontSize) };
		const int textY{ centerY - fontSize / 2 + static_cast<int>(scaled(READY_RISE) * riseRate) };

		m_uiRenderer.setFont(MONO_FONT_NAME);
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA,
		    static_cast<int>(255 * alphaRate));
		m_uiRenderer.drawText((screenWidth - textWidth) / 2, textY, READY_TEXT,
		    core::utility::Color::HUD_INK, fontSize);
		m_uiRenderer.resetBlendMode();
		m_uiRenderer.resetFont();
	}

	void BattleStartSystem::drawFight()
	{
		const float fightTime{ m_elapsedTime - FIGHT_TIME };

		// ホールドの間は振り切ったまま、そのあとフェードアウトする
		const float alphaRate{ fightTime < FIGHT_HOLD
			                       ? 1.0f
			                       : 1.0f - smoothstep((fightTime - FIGHT_HOLD) / FIGHT_FADE_OUT) };
		if (alphaRate <= 0.0f)
			return;

		const int screenWidth{ m_screen.getWidth() };
		const int screenHeight{ m_screen.getHeight() };
		const int centerX{ screenWidth / 2 };
		const int centerY{ screenHeight * TEXT_CENTER_Y_RATIO_PERCENT / 100 };

		// 出た瞬間の白いフラッシュ（画面全体）。加算で一瞬だけ飛ばす
		if (fightTime < FLASH_TIME)
		{
			const int flashAlpha{ static_cast<int>(FLASH_ALPHA * (1.0f - fightTime / FLASH_TIME)) };
			m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ADD, flashAlpha);
			m_uiRenderer.drawBox(0, 0, screenWidth, screenHeight, core::utility::Color::WHITE, true);
		}

		// フェードアウトしながら浮き上がらせる（消え際に上へ抜ける）
		const int fontSize{ scaled(FIGHT_FONT_SIZE) };
		const int textWidth{ m_uiRenderer.getTextWidth(FIGHT_TEXT, fontSize) };
		const int rise{ static_cast<int>(scaled(FIGHT_RISE) * (1.0f - alphaRate)) };
		const int textY{ centerY - fontSize / 2 - rise };

		m_uiRenderer.setFont(MONO_FONT_NAME);
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA,
		    static_cast<int>(255 * alphaRate));
		m_uiRenderer.drawText(centerX - textWidth / 2, textY, FIGHT_TEXT,
		    core::utility::Color::HUD_CHARGE_CYAN, fontSize);
		m_uiRenderer.resetBlendMode();
		m_uiRenderer.resetFont();

		// 文字の下を左右へ走る発光ライン。文字だけより「始まった」という勢いが出る
		const float expandRate{ smoothstep(fightTime / LINE_EXPAND_TIME) };
		const int halfWidth{ static_cast<int>(scaled(LINE_HALF_WIDTH) * expandRate) };
		if (halfWidth <= 0)
			return;

		const int lineY{ textY + fontSize + scaled(LINE_GAP) };
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ADD,
		    static_cast<int>(255 * alphaRate));
		m_uiRenderer.drawBox(centerX - halfWidth, lineY, halfWidth * 2, scaled(LINE_THICKNESS),
		    core::utility::Color::HUD_ACCENT, true);
		m_uiRenderer.resetBlendMode();
	}
} // namespace game::system::visual
