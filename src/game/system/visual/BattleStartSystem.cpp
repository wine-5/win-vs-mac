#include "BattleStartSystem.h"
#include "game/component/movement/InputComponent.h"
#include "game/component/ai/AIComponent.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"
#include "core/constant/SeType.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IAudioManager.h"
#include "core/interface/IStringConverter.h"
#include "core/utility/Easing.h"
#include "core/utility/MathConstants.h"
#include "core/utility/Probe.h" // 一時: メモリ調査用（原因特定後に削除）
#include <algorithm>
#include <cmath>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// --- ミッション提示のタイムライン（秒） ---
	constexpr float MISSION_FADE_IN{ 0.40f };     // ミッションのカードが浮かび上がる
	constexpr float MISSION_PROMPT_DELAY{ 2.0f }; // 「Enterで開始」が出るまでの間（読む時間）
	constexpr float MISSION_PROMPT_FADE_IN{ 0.35f };
	constexpr float MISSION_FLY_TIME{ 0.55f }; // 中央から左上へ流れ着くまでの時間

	// 流れ着く先（左上のObjectiveViewのパネル中心・1080p基準）。
	// ObjectiveView側のPANEL_MARGIN/WIDTH/HEIGHTと同じ値から中心を求めている
	constexpr int OBJECTIVE_PANEL_MARGIN{ 28 };
	constexpr int OBJECTIVE_PANEL_WIDTH{ 330 };
	constexpr int OBJECTIVE_PANEL_HEIGHT{ 104 };

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

	// 「Windowsの内部」という世界観に合わせ、システムメッセージのような等幅＋字間で見せる
	constexpr const char* READY_TEXT{ "R E A D Y" };
	constexpr const char* FIGHT_TEXT{ "F I G H T !" };

	// ミッションの文面。左上のObjectiveViewと同じ目標を、初見でも分かる言い回しで先に伝える
	constexpr const char* MISSION_CAPTION{ "MISSION" };
	constexpr const char* MISSION_TEXT{ "敵をすべて撃破せよ" };
	constexpr const char* MISSION_DETAIL_TEXT{ "全滅させると現れるボス「Mac」も撃破せよ" };
	constexpr const char* MISSION_PROMPT_TEXT{ "クリック / Enter / Space で開始" };

	// ミッションのカード（中央・1080p基準）
	constexpr int MISSION_CARD_WIDTH{ 1000 };
	constexpr int MISSION_CARD_HEIGHT{ 250 };
	constexpr int MISSION_CAPTION_FONT_SIZE{ 24 };
	constexpr int MISSION_FONT_SIZE{ 60 };
	constexpr int MISSION_DETAIL_FONT_SIZE{ 27 };
	constexpr int MISSION_CAPTION_OFFSET_Y{ -92 }; // カード中心からの相対位置
	constexpr int MISSION_TEXT_OFFSET_Y{ -34 };
	constexpr int MISSION_DETAIL_OFFSET_Y{ 50 };
	constexpr int MISSION_CARD_ALPHA{ 205 };
	constexpr int MISSION_ACCENT_THICKNESS{ 3 }; // 見出しの下に引くアクセント線

	// プロンプト（カードの下）。明るい床の上でも読めるよう、暗い帯を敷いた上に載せる
	constexpr int MISSION_PROMPT_FONT_SIZE{ 30 };
	constexpr int MISSION_PROMPT_GAP{ 40 }; // カード下端から帯の上端までの間隔
	constexpr int MISSION_PROMPT_PADDING_X{ 32 };
	constexpr int MISSION_PROMPT_PADDING_Y{ 14 };
	constexpr int MISSION_PROMPT_BAND_ALPHA{ 200 };
	constexpr int MISSION_PROMPT_BAND_MIN_ALPHA{ 60 };
	constexpr float MISSION_PROMPT_BLINK_CYCLE{ 1.8f };
	constexpr int MISSION_PROMPT_MIN_ALPHA{ 70 };

	// 流れていく間の縮小率（流れ着いた時点の大きさ）
	constexpr float MISSION_FLY_END_SCALE{ 0.42f };

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

} // namespace

namespace game::system::visual
{
	BattleStartSystem::BattleStartSystem(core::ecs::ComponentManager& componentManager,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::iface::IInputProvider& inputProvider,
	    core::ecs::EntityId playerId)
	    : m_componentManager{ componentManager }
	    , m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_inputProvider{ inputProvider }
	    , m_playerId{ playerId }
	    , m_missionText{ MISSION_TEXT }
	    , m_missionDetailText{ MISSION_DETAIL_TEXT }
	    , m_promptText{ MISSION_PROMPT_TEXT }
	{
		// Systemの登録順に関係なく初回フレームから止めたいので、update待ちではなくここでロックする
		setGameplayLocked(true);

		// DxLibの描画はShift_JISを期待する。ソース上のUTF-8をそのまま渡すと日本語が化けるため、
		// ここで一度だけ変換しておく
		if (auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() })
		{
			m_missionText = converter->utf8ToShiftJis(m_missionText);
			m_missionDetailText = converter->utf8ToShiftJis(m_missionDetailText);
			m_promptText = converter->utf8ToShiftJis(m_promptText);
		}

		// ミッション提示の出だしに鳴らす。READYの合図と同じ音で「これから始まる」を揃える
		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio)
			audio->playSe(core::constant::SeType::BattleReady);
	}

	bool BattleStartSystem::isPreparing() const noexcept
	{
		return m_isLocked;
	}

	bool BattleStartSystem::isObjectiveRevealed() const noexcept
	{
		return m_phase == Phase::Battle;
	}

	bool BattleStartSystem::isAdvanceRequested()
	{
		const bool isMouseLeftDown{ m_inputProvider.isMouseLeftPressed() };
		const bool isMouseLeftClicked{ isMouseLeftDown && !m_wasMouseLeftDown };
		m_wasMouseLeftDown = isMouseLeftDown;

		return m_inputProvider.isKeyPressed(core::input::KeyCode::Enter) ||
		       m_inputProvider.isKeyPressed(core::input::KeyCode::Space) ||
		       m_inputProvider.isPadButtonPressed(core::input::GamePadCode::ButtonCross) ||
		       isMouseLeftClicked;
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

		m_phaseTime += deltaTime;

		// ミッションを読み終えるまでREADYへ進まない。フェードインの途中で飛ばすと
		// 何が出たのか分からないまま消えるため、出し切ってから入力を受け付ける
		if (m_phase == Phase::Mission)
		{
			if (m_phaseTime >= MISSION_FADE_IN && isAdvanceRequested())
			{
				m_phase = Phase::Fly;
				m_phaseTime = 0.0f;
			}
			return;
		}

		// 左上へ流れ着いたらREADYを始める。ここでObjectiveViewの表示も解禁される
		if (m_phase == Phase::Fly)
		{
			if (m_phaseTime >= MISSION_FLY_TIME)
			{
				m_phase = Phase::Battle;
				m_phaseTime = 0.0f;
			}
			return;
		}

		m_elapsedTime += deltaTime;

		// FIGHT!が出た瞬間に操作と敵AIを解禁する。文字が消えるのを待たせると、
		// もう動けるのか分からない空白の時間ができる
		if (m_isLocked && m_elapsedTime >= FIGHT_TIME)
		{
			// 一時: 敵AI解禁の前後を挟んで、解禁そのものが確保しているのかを見る（原因特定後に削除）
			core::probe::mark("  カウントダウン: 解禁 前");
			setGameplayLocked(false);
			core::probe::mark("  カウントダウン: 解禁 後");

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

		if (m_phase == Phase::Mission)
		{
			drawMission();
			return;
		}

		if (m_phase == Phase::Fly)
		{
			drawMissionFly();
			return;
		}

		if (m_elapsedTime < FIGHT_TIME)
			drawReady();
		else
			drawFight();
	}

	void BattleStartSystem::drawMissionCard(int centerX, int centerY, float scale, float alphaRate)
	{
		if (alphaRate <= 0.0f)
			return;

		auto scaledBy = [&](int value)
		{ return static_cast<int>(scaled(value) * scale); };

		const int cardWidth{ scaledBy(MISSION_CARD_WIDTH) };
		const int cardHeight{ scaledBy(MISSION_CARD_HEIGHT) };
		const int cardX{ centerX - cardWidth / 2 };
		const int cardY{ centerY - cardHeight / 2 };

		// 下地。HudPanelは濃さを引数に取らないので、ブレンドを掛けた状態で描かせる
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA,
		    static_cast<int>(MISSION_CARD_ALPHA * alphaRate));
		m_uiRenderer.drawBox(cardX, cardY, cardWidth, cardHeight, core::utility::Color::BLACK, true);

		// フォントは文字サイズごとにハンドルが作られ、1つあたり数MBを確保する。
		// 拡縮アニメの途中でサイズを1pxずつ変えると1フレームごとに別のフォントが増えるため、
		// 段階を粗くしてハンドルの種類を抑える
		auto fontSizeOf = [&](int value)
		{
			constexpr int FONT_SIZE_STEP{ 8 };
			const int raw{ scaledBy(value) };
			return std::max(FONT_SIZE_STEP, (raw + FONT_SIZE_STEP / 2) / FONT_SIZE_STEP * FONT_SIZE_STEP);
		};

		const int textAlpha{ static_cast<int>(255 * alphaRate) };
		const int captionFontSize{ fontSizeOf(MISSION_CAPTION_FONT_SIZE) };
		const int missionFontSize{ fontSizeOf(MISSION_FONT_SIZE) };
		const int detailFontSize{ fontSizeOf(MISSION_DETAIL_FONT_SIZE) };

		// 見出し（等幅）＋その下のアクセント線。「システムからの指令」という体裁にする
		m_uiRenderer.setFont(core::constant::ui::MONO_SEMIBOLD_FONT_NAME);
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, textAlpha);
		const int captionWidth{ m_uiRenderer.getTextWidth(MISSION_CAPTION, captionFontSize) };
		const int captionY{ centerY + scaledBy(MISSION_CAPTION_OFFSET_Y) };
		m_uiRenderer.drawText(centerX - captionWidth / 2, captionY, MISSION_CAPTION,
		    core::utility::Color::HUD_ACCENT, captionFontSize);
		m_uiRenderer.resetFont();

		const int accentThickness{ std::max(1, scaledBy(MISSION_ACCENT_THICKNESS)) };
		m_uiRenderer.drawBox(centerX - captionWidth / 2, captionY + captionFontSize + accentThickness,
		    captionWidth, accentThickness, core::utility::Color::HUD_ACCENT, true);

		// 本文と補足
		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
		const int missionWidth{ m_uiRenderer.getTextWidth(m_missionText.c_str(), missionFontSize) };
		m_uiRenderer.drawText(centerX - missionWidth / 2, centerY + scaledBy(MISSION_TEXT_OFFSET_Y),
		    m_missionText.c_str(), core::utility::Color::HUD_INK, missionFontSize);

		const int detailWidth{ m_uiRenderer.getTextWidth(m_missionDetailText.c_str(), detailFontSize) };
		m_uiRenderer.drawText(centerX - detailWidth / 2, centerY + scaledBy(MISSION_DETAIL_OFFSET_Y),
		    m_missionDetailText.c_str(), core::utility::Color::HUD_INK_FAINT, detailFontSize);
		m_uiRenderer.resetFont();
		m_uiRenderer.resetBlendMode();
	}

	void BattleStartSystem::drawMission()
	{
		const float alphaRate{ core::utility::smoothstep(m_phaseTime / MISSION_FADE_IN) };
		const int centerX{ m_screen.getWidth() / 2 };
		const int centerY{ m_screen.getHeight() * TEXT_CENTER_Y_RATIO_PERCENT / 100 };

		drawMissionCard(centerX, centerY, 1.0f, alphaRate);

		// 操作プロンプトは一拍おいてから出す。カードと同時に出すと目線が下へ流れ、
		// 肝心のミッションを読まないまま進まれてしまう
		if (m_phaseTime < MISSION_PROMPT_DELAY)
			return;

		const float promptTime{ m_phaseTime - MISSION_PROMPT_DELAY };
		const float promptFade{ core::utility::smoothstep(promptTime / MISSION_PROMPT_FADE_IN) };

		// ゆっくりフェードイン・フェードアウトを繰り返して「入力を待っている」ことを示す。
		// 帯も文字と一緒に濃さを変える（帯だけ残ると黒い箱が貼り付いて見える）
		const float blink{ 0.5f + 0.5f * std::cos(promptTime / MISSION_PROMPT_BLINK_CYCLE * core::utility::TWO_PI) };
		const int promptAlpha{ static_cast<int>(
			(MISSION_PROMPT_MIN_ALPHA + (255 - MISSION_PROMPT_MIN_ALPHA) * blink) * promptFade) };
		const int bandAlpha{ static_cast<int>(
			(MISSION_PROMPT_BAND_MIN_ALPHA +
			    (MISSION_PROMPT_BAND_ALPHA - MISSION_PROMPT_BAND_MIN_ALPHA) * blink) *
			promptFade) };

		const int promptFontSize{ scaled(MISSION_PROMPT_FONT_SIZE) };

		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
		const int promptWidth{ m_uiRenderer.getTextWidth(m_promptText.c_str(), promptFontSize) };

		// 床が明るいステージでは文字だけだと沈む。文字の後ろに暗い帯を敷いて必ず読めるようにする
		const int paddingX{ scaled(MISSION_PROMPT_PADDING_X) };
		const int paddingY{ scaled(MISSION_PROMPT_PADDING_Y) };
		const int bandWidth{ promptWidth + paddingX * 2 };
		const int bandHeight{ promptFontSize + paddingY * 2 };
		const int bandY{ centerY + scaled(MISSION_CARD_HEIGHT) / 2 + scaled(MISSION_PROMPT_GAP) };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, bandAlpha);
		m_uiRenderer.drawBox(centerX - bandWidth / 2, bandY, bandWidth, bandHeight,
		    core::utility::Color::BLACK, true);

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, promptAlpha);
		m_uiRenderer.drawText(centerX - promptWidth / 2, bandY + paddingY, m_promptText.c_str(),
		    core::utility::Color::HUD_CHARGE_CYAN, promptFontSize);
		m_uiRenderer.resetBlendMode();
		m_uiRenderer.resetFont();
	}

	void BattleStartSystem::drawMissionFly()
	{
		const float progress{ core::utility::smoothstep(m_phaseTime / MISSION_FLY_TIME) };

		const int startX{ m_screen.getWidth() / 2 };
		const int startY{ m_screen.getHeight() * TEXT_CENTER_Y_RATIO_PERCENT / 100 };

		// 流れ着く先は左上のObjectiveViewのパネル中心。着いた瞬間に本物と入れ替わる
		const int endX{ scaled(OBJECTIVE_PANEL_MARGIN) + scaled(OBJECTIVE_PANEL_WIDTH) / 2 };
		const int endY{ scaled(OBJECTIVE_PANEL_MARGIN) + scaled(OBJECTIVE_PANEL_HEIGHT) / 2 };

		const int centerX{ startX + static_cast<int>((endX - startX) * progress) };
		const int centerY{ startY + static_cast<int>((endY - startY) * progress) };
		const float scale{ 1.0f + (MISSION_FLY_END_SCALE - 1.0f) * progress };

		// 着地の手前で薄くしていき、ObjectiveViewへ自然に引き継ぐ
		drawMissionCard(centerX, centerY, scale, 1.0f - progress * progress);
	}

	void BattleStartSystem::drawReady()
	{
		// フェードイン→ホールド→フェードアウトの濃さ（0〜1）を求める
		float alphaRate{ 1.0f };
		if (m_elapsedTime < READY_FADE_IN)
			alphaRate = core::utility::smoothstep(m_elapsedTime / READY_FADE_IN);
		else if (m_elapsedTime >= READY_FADE_IN + READY_HOLD)
			alphaRate = 1.0f - core::utility::smoothstep((m_elapsedTime - READY_FADE_IN - READY_HOLD) / READY_FADE_OUT);

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
		const float riseRate{ 1.0f - core::utility::smoothstep(m_elapsedTime / READY_FADE_IN) };
		const int fontSize{ scaled(READY_FONT_SIZE) };
		const int textWidth{ m_uiRenderer.getTextWidth(READY_TEXT, fontSize) };
		const int textY{ centerY - fontSize / 2 + static_cast<int>(scaled(READY_RISE) * riseRate) };

		m_uiRenderer.setFont(core::constant::ui::MONO_SEMIBOLD_FONT_NAME);
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
			                       : 1.0f - core::utility::smoothstep((fightTime - FIGHT_HOLD) / FIGHT_FADE_OUT) };
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

		m_uiRenderer.setFont(core::constant::ui::MONO_SEMIBOLD_FONT_NAME);
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA,
		    static_cast<int>(255 * alphaRate));
		// 開戦の合図は青い床の上でも沈まないオレンジで出す。
		// READY（白）→ FIGHT!（オレンジ）と色を変えることで、切り替わった瞬間も分かりやすい
		m_uiRenderer.drawText(centerX - textWidth / 2, textY, FIGHT_TEXT,
		    core::utility::Color::HUD_CRITICAL_ORANGE, fontSize);
		m_uiRenderer.resetBlendMode();
		m_uiRenderer.resetFont();

		// 文字の下を左右へ走る発光ライン。文字だけより「始まった」という勢いが出る
		const float expandRate{ core::utility::smoothstep(fightTime / LINE_EXPAND_TIME) };
		const int halfWidth{ static_cast<int>(scaled(LINE_HALF_WIDTH) * expandRate) };
		if (halfWidth <= 0)
			return;

		const int lineY{ textY + fontSize + scaled(LINE_GAP) };
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ADD,
		    static_cast<int>(255 * alphaRate));
		m_uiRenderer.drawBox(centerX - halfWidth, lineY, halfWidth * 2, scaled(LINE_THICKNESS),
		    core::utility::Color::HUD_CRITICAL_ORANGE, true);
		m_uiRenderer.resetBlendMode();
	}
} // namespace game::system::visual
