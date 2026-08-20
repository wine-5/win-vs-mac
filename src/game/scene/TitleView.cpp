#include "TitleView.h"
#include "core/base/ServiceLocator.h"
#include "core/constant/UI.h"
#include "core/interface/IStringConverter.h"
#include "core/utility/Color.h"
#include "core/utility/Easing.h"
#include "core/utility/MathConstants.h"
#include "core/constant/SeType.h"
#include "core/interface/IAudioManager.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace
{
	using Color = core::utility::Color;

	/** @brief アプリアイコンのリソースID */
	constexpr const char* APP_ICON_IMAGE_ID{ "game-icon" };

	// ウィンドウは画面いっぱいに広げる（最大化したタスクマネージャーと同じ状態）。
	// 中の寸法はすべてウィンドウ高さから換算するので、画面が大きいほど文字も一緒に拡大する。

	// 以下は基準サイズ（ウィンドウ高さ740px）でのピクセル数
	constexpr float BASE_WINDOW_HEIGHT{ 740.0f };

	constexpr float TITLE_BAR_HEIGHT{ 48.0f };
	constexpr float NAV_WIDTH{ 226.0f };
	constexpr float NAV_LEFT_MARGIN{ 12.0f };
	constexpr float NAV_RIGHT_MARGIN{ 16.0f };
	constexpr float NAV_ITEM_HEIGHT{ 42.0f };
	constexpr float NAV_TEXT_INDENT{ 44.0f };
	constexpr float NAV_PILL_WIDTH{ 3.0f };
	constexpr float NAV_PILL_HEIGHT{ 16.0f };
	constexpr float NAV_TOP_GAP{ 48.0f };    // ハンバーガーの下から最初の項目まで
	constexpr float NAV_BOTTOM_GAP{ 12.0f }; // 左下の「設定」の下余白
	constexpr float NAV_ICON_SIZE{ 18.0f };  // 項目のアイコンの一辺
	constexpr float NAV_ICON_LEFT{ 12.0f };  // 項目の左端からアイコンの左端まで

	// 歯車の各部を「アイコンの一辺」に対する比率で持つ。拡大しても形が崩れないようにする
	constexpr float GEAR_RING_RADIUS_RATIO{ 0.29f };    // 輪の中心線の半径
	constexpr float GEAR_RING_THICKNESS_RATIO{ 0.13f }; // 輪の太さ
	constexpr float GEAR_TOOTH_LENGTH_RATIO{ 0.47f };   // 中心から歯の先端まで
	constexpr int GEAR_TOOTH_COUNT{ 8 };

	constexpr float CONTENT_PADDING_X{ 28.0f };
	constexpr float CONTENT_PADDING_TOP{ 22.0f };
	constexpr float CONTENT_PADDING_BOTTOM{ 20.0f };

	constexpr float APP_ICON_SIZE{ 56.0f };
	constexpr float APP_ICON_GAP{ 16.0f };
	constexpr float APP_HEADER_HEIGHT{ 78.0f }; // アイコンの高さ＋下の余白

	constexpr float THUMB_COLUMN_WIDTH{ 234.0f };
	constexpr float THUMB_HEIGHT{ 78.0f };
	constexpr float THUMB_GAP{ 10.0f };
	constexpr float THUMB_GRAPH_WIDTH{ 98.0f };
	constexpr float THUMB_GRAPH_HEIGHT{ 60.0f };
	constexpr float THUMB_PADDING{ 8.0f };
	constexpr float THUMB_TEXT_GAP{ 12.0f };
	constexpr float PANES_GAP{ 22.0f };

	constexpr float DETAIL_HEAD_HEIGHT{ 46.0f };
	constexpr float GRAPH_CAPTION_HEIGHT{ 22.0f };
	constexpr float GRAPH_AXIS_HEIGHT{ 22.0f };
	constexpr float STATS_HEIGHT{ 60.0f };
	constexpr float STATS_TOP_GAP{ 16.0f };
	constexpr float STATS_COLUMN_WIDTH{ 152.0f };

	constexpr float BUTTON_HEIGHT{ 38.0f };
	constexpr float EXIT_BUTTON_WIDTH{ 180.0f };
	constexpr float START_BUTTON_WIDTH{ 200.0f };
	constexpr float START_BUTTON_HEIGHT{ 48.0f };
	constexpr float START_BUTTON_TOP_GAP{ 14.0f };
	constexpr float BUTTON_RADIUS{ 4.0f };

	// 「選択画面へ」から広がる輪。押さないとゲームが始まらないボタンなので、
	// 間を空けて一度だけ広げる。出しっぱなしにすると画面が落ち着かない
	constexpr float PULSE_INTERVAL{ 2.4f };    // 次の輪が出るまでの周期（秒）
	constexpr float PULSE_DURATION{ 0.9f };    // 輪が広がりきるまで（秒）
	constexpr float PULSE_MAX_SPREAD{ 14.0f }; // ボタンの外へ広がる幅
	constexpr float PULSE_THICKNESS{ 2.0f };   // 輪の太さ
	constexpr float PULSE_MAX_ALPHA{ 170.0f }; // 出はじめの濃さ

	// 起動演出。完成形がいきなり出ると静止画に見えるので、実物のウィンドウが開くときと同じ
	// 「枠が出る → 中身が下から入る」順で組み立てる。数字はすべて秒
	constexpr float INTRO_VEIL_DURATION{ 0.30f }; // 地の色の覆いが引くまで
	constexpr float INTRO_HEADER_START{ 0.22f };  // ゲーム名
	constexpr float INTRO_HEADER_DURATION{ 0.50f };
	constexpr float INTRO_PANEL_START{ 0.46f }; // サムネイルとグラフ
	constexpr float INTRO_PANEL_DURATION{ 0.52f };
	constexpr float INTRO_BUTTON_START{ 0.90f }; // 2つのボタン
	constexpr float INTRO_BUTTON_DURATION{ 0.34f };
	constexpr float INTRO_TOTAL{ INTRO_BUTTON_START + INTRO_BUTTON_DURATION };

	/** @brief 起動演出でせり上がる距離（基準サイズでのピクセル数） */
	constexpr float INTRO_SLIDE{ 16.0f };

	/** @brief コンテンツ面の角丸。最大化したウィンドウで丸くなるのは左上だけ */
	constexpr float CONTENT_RADIUS{ 8.0f };
	constexpr float GRID_LINE_COUNT{ 10.0f };

	/** @brief グラフの塗りの濃さ。白地なので、線が読める程度まで薄くする */
	constexpr int GRAPH_FILL_ALPHA{ 64 };

	// フォントサイズ（基準サイズでのピクセル数）。
	// 実物は手元で読む前提の細かい字だが、こちらは離れて見るゲーム画面なので一回り大きくする
	constexpr float FONT_TITLE_BAR{ 15.0f };
	constexpr float FONT_GAME_TITLE{ 44.0f };
	constexpr float FONT_APP_SUB{ 15.0f };
	constexpr float FONT_NAV{ 16.0f };
	constexpr float FONT_THUMB_NAME{ 17.0f };
	constexpr float FONT_THUMB_VALUE{ 14.0f };
	constexpr float FONT_CHANNEL_NAME{ 30.0f };
	constexpr float FONT_SMALL{ 14.0f };
	constexpr float FONT_STAT_LABEL{ 14.0f };
	constexpr float FONT_STAT_VALUE{ 30.0f };
	constexpr float FONT_BUTTON{ 15.0f };
	constexpr float FONT_START_BUTTON{ 17.0f };

	/** @brief チャンネルごとの見せ方。値そのものは TitleView が持つ */
	struct ChannelSpec
	{
		const char* m_name;      // サムネイルと見出しに出す名前
		const char* m_caption;   // グラフの上に出す説明
		const char* m_statLabel; // 詳細値の1つ目のラベル
		unsigned int m_color;
		float m_smoothFactor; // EMAの係数（0に近いほど滑らか）
	};

	// 並びは TitleChannel と対にする
	constexpr ChannelSpec CHANNEL_SPECS[]{
		{ "CPU", "CPU 使用率", "使用率", Color::TITLE_GRAPH_CPU, 0.15f },
		{ "メモリ", "メモリ使用率", "使用率", Color::TITLE_GRAPH_MEMORY, 0.40f },
		{ "ディスク", "ディスクのアクティブな時間", "アクティブな時間", Color::TITLE_GRAPH_DISK, 0.15f },
	};
	// キー・パッドで操作しているときに出す選択枠（基準サイズでのピクセル数）
	constexpr float FOCUS_RING_MARGIN{ 3.0f };
	constexpr float FOCUS_RING_RADIUS{ 6.0f };
	constexpr float FOCUS_RING_THICKNESS{ 2.0f };

} // namespace

namespace game::scene
{
	TitleView::TitleView(core::iface::IInputProvider& inputProvider,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::iface::IResourceManager& resourceManager,
	    std::function<void()> onGoToSelect,
	    std::function<void()> onOpenSettings,
	    std::function<void()> onExit)
	    : m_inputProvider{ inputProvider }
	    , m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_inputMapper{ inputProvider }
	    , m_onGoToSelect{ std::move(onGoToSelect) }
	    , m_onOpenSettings{ std::move(onOpenSettings) }
	    , m_onExit{ std::move(onExit) }
	{
		// 読み込めなくてもアイコンを描かないだけで、画面は成立する
		m_iconHandle = resourceManager.loadImageById(APP_ICON_IMAGE_ID);
	}

	void TitleView::setButtonsVisible(bool visible)
	{
		m_isInteractive = visible;

		// 受け付け始めた瞬間のクリックで誤爆しないよう、現在の押下状態を引き継ぐ
		if (!visible)
			return;

		m_prevMouseLeft = m_inputProvider.isMouseLeftPressed();

		// 前の画面から押しっぱなしのキーをそのまま拾わないよう、枠の状態も作り直す
		m_inputMapper.reset();
	}

	void TitleView::pushHistory(std::array<float, HISTORY_SIZE>& buffer, float value)
	{
		std::ranges::rotate(buffer, buffer.begin() + 1);
		buffer.back() = value;
	}

	void TitleView::update(const core::iface::PerformanceSnapshot& snap, float deltaTime)
	{
		if (!isIntroFinished())
			m_introTimer += deltaTime;

		m_pulseTimer += deltaTime;
		if (m_pulseTimer >= PULSE_INTERVAL)
			m_pulseTimer -= PULSE_INTERVAL;

		const float rawValues[CHANNEL_COUNT]{ snap.cpuUsage, snap.memoryUsage, snap.diskActivity };

		for (int i{ 0 }; i < CHANNEL_COUNT; ++i)
		{
			// 生値のままだと折れ線が毎フレーム飛ぶので、EMAで均してから積む
			ChannelState& channel{ m_channels[i] };
			channel.m_smoothed += CHANNEL_SPECS[i].m_smoothFactor * (rawValues[i] - channel.m_smoothed);
			pushHistory(channel.m_history, channel.m_smoothed);
		}

		updateLayout();

		int mouseX{}, mouseY{};
		m_inputProvider.getMousePosition(mouseX, mouseY);
		// まだ出ていないものは押せない。起動演出が終わってから受け付ける
		m_hovered = (m_isInteractive && isIntroFinished()) ? getHitAt(mouseX, mouseY) : Hit::None;

		const bool mouseLeft{ m_inputProvider.isMouseLeftPressed() };
		const bool mouseClicked{ mouseLeft && !m_prevMouseLeft };
		m_prevMouseLeft = mouseLeft;

		// キー・パッドの操作を読む。演出中は受け付けないが、読むこと自体は毎フレーム行う
		// （読み飛ばすと、受け付け始めた瞬間に溜まっていた入力が一度に効く）
		m_inputMapper.update(deltaTime);

		if (!m_isInteractive || !isIntroFinished())
			return;

		if (m_inputMapper.isTriggered(ui::UiAction::NavigateUp))
			moveFocus(0, -1);
		if (m_inputMapper.isTriggered(ui::UiAction::NavigateDown))
			moveFocus(0, 1);
		if (m_inputMapper.isTriggered(ui::UiAction::NavigateLeft))
			moveFocus(-1, 0);
		if (m_inputMapper.isTriggered(ui::UiAction::NavigateRight))
			moveFocus(1, 0);

		if (m_inputMapper.isTriggered(ui::UiAction::Confirm))
			activate(focusedHit());

		if (mouseClicked)
			activate(m_hovered);
	}

	TitleView::Hit TitleView::focusedHit() const noexcept
	{
		return FOCUS_ORDER[m_focusColumn][m_focusRow];
	}

	void TitleView::moveFocus(int columnDelta, int rowDelta) noexcept
	{
		const int previousColumn{ m_focusColumn };
		const int previousRow{ m_focusRow };

		if (rowDelta != 0)
		{
			// 端で止める。回り込ませると、下端から一気に上端へ飛んで位置を見失う
			m_focusRow = std::clamp(m_focusRow + rowDelta, 0, FOCUS_ROW_COUNT[m_focusColumn] - 1);
		}
		else if (columnDelta != 0)
		{
			const int nextColumn{ std::clamp(m_focusColumn + columnDelta, 0, FOCUS_COLUMN_COUNT - 1) };

			// 列を移るときは画面上で近いほうへ着地させる。行番号をそのまま持ち越すと、
			// 右下のボタンから左へ移ったのに真ん中のサムネイルへ飛ぶ、といったことが起きる
			const bool isUpperHalf{ m_focusRow * 2 < FOCUS_ROW_COUNT[m_focusColumn] };
			m_focusColumn = nextColumn;
			m_focusRow = isUpperHalf ? 0 : FOCUS_ROW_COUNT[nextColumn] - 1;
		}

		// 実際に動いたときだけ鳴らす。端で止まっているのに鳴り続けると、
		// 動いていないのか音だけ鳴っているのか分からなくなる
		if (m_focusColumn == previousColumn && m_focusRow == previousRow)
			return;

		if (auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() })
			audio->playSe(core::constant::SeType::UiKeyPress);
	}

	bool TitleView::isHighlighted(Hit hit) const noexcept
	{
		if (hit == Hit::None)
			return false;
		if (m_hovered == hit)
			return true;

		return m_inputMapper.isFocusVisible() && focusedHit() == hit;
	}

	void TitleView::activate(Hit hit)
	{
		switch (hit)
		{
		case Hit::ThumbCpu:
			m_selectedChannel = static_cast<int>(TitleChannel::Cpu);
			playUiClick();
			break;
		case Hit::ThumbMemory:
			m_selectedChannel = static_cast<int>(TitleChannel::Memory);
			playUiClick();
			break;
		case Hit::ThumbDisk:
			m_selectedChannel = static_cast<int>(TitleChannel::Disk);
			playUiClick();
			break;

		// 3つのボタンは押した先（Title::goToSelect など）が鳴らすので、ここでは鳴らさない
		case Hit::Settings:
			if (m_onOpenSettings)
				m_onOpenSettings();
			break;
		case Hit::Exit:
			if (m_onExit)
				m_onExit();
			break;
		case Hit::Start:
			if (m_onGoToSelect)
				m_onGoToSelect();
			break;
		default: break;
		}
	}

	bool TitleView::getHitRect(Hit hit, int& outX, int& outY, int& outWidth, int& outHeight) const
	{
		switch (hit)
		{
		case Hit::ThumbCpu: getThumbRect(0, outX, outY, outWidth, outHeight); return true;
		case Hit::ThumbMemory: getThumbRect(1, outX, outY, outWidth, outHeight); return true;
		case Hit::ThumbDisk: getThumbRect(2, outX, outY, outWidth, outHeight); return true;
		case Hit::Settings: getNavSettingsRect(outX, outY, outWidth, outHeight); return true;
		case Hit::Exit: getExitButtonRect(outX, outY, outWidth, outHeight); return true;
		case Hit::Start: getStartButtonRect(outX, outY, outWidth, outHeight); return true;
		default: return false;
		}
	}

	void TitleView::drawFocusRing() const
	{
		if (!m_isInteractive || !isIntroFinished() || !m_inputMapper.isFocusVisible())
			return;

		int x{}, y{}, width{}, height{};
		if (!getHitRect(focusedHit(), x, y, width, height))
			return;

		// 対象より一回り外へ描く。枠の内側に重ねると、ボタン自身の縁と混ざって
		// どちらが選択の印なのか分からなくなる
		const int margin{ scaled(FOCUS_RING_MARGIN) };
		m_uiRenderer.drawRoundedBox(x - margin, y - margin, width + margin * 2,
		    height + margin * 2, scaled(FOCUS_RING_RADIUS),
		    core::utility::Color::TITLE_ACCENT, false, scaled(FOCUS_RING_THICKNESS));
	}

	void TitleView::playUiClick() const
	{
		if (auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() })
			audio->playSe(core::constant::SeType::UiClick);
	}

	int TitleView::scaled(float basePixels) const noexcept
	{
		return std::max(1, static_cast<int>(basePixels * m_scale));
	}

	bool TitleView::isIntroFinished() const noexcept
	{
		return m_introTimer >= INTRO_TOTAL;
	}

	float TitleView::introProgress(float start, float duration) const noexcept
	{
		return std::clamp((m_introTimer - start) / duration, 0.0f, 1.0f);
	}

	int TitleView::introAlpha(float start, float duration) const noexcept
	{
		// 両端を緩めると、点いたり消えたりではなく「浮かび上がる」ように見える
		return static_cast<int>(255.0f * core::utility::smoothstep(introProgress(start, duration)));
	}

	int TitleView::introSlide(float start, float duration) const noexcept
	{
		// scaled() は最低1を返すので、演出後に1pxずれ続けないようここで直に換算する
		const float remaining{ 1.0f - core::utility::easeOut(introProgress(start, duration)) };
		return static_cast<int>(INTRO_SLIDE * m_scale * remaining);
	}

	void TitleView::updateLayout()
	{
		const int screenWidth{ m_screen.getWidth() };
		const int screenHeight{ m_screen.getHeight() };

		// 画面いっぱいに広げる。実物を最大化したときと同じ収まりにする
		m_windowWidth = screenWidth;
		m_windowHeight = screenHeight;
		m_windowX = 0;
		m_windowY = 0;

		m_scale = m_windowHeight / BASE_WINDOW_HEIGHT;

		m_titleBarHeight = scaled(TITLE_BAR_HEIGHT);
		m_navWidth = scaled(NAV_WIDTH);

		m_contentX = m_windowX + m_navWidth + scaled(CONTENT_PADDING_X);
		m_contentY = m_windowY + m_titleBarHeight + scaled(CONTENT_PADDING_TOP);
		m_contentWidth = m_windowWidth - m_navWidth - scaled(CONTENT_PADDING_X) * 2;
		m_contentHeight = m_windowHeight - m_titleBarHeight - scaled(CONTENT_PADDING_TOP) - scaled(CONTENT_PADDING_BOTTOM);

		m_panesTop = m_contentY + scaled(APP_HEADER_HEIGHT);

		const int thumbColumn{ scaled(THUMB_COLUMN_WIDTH) };
		m_detailX = m_contentX + thumbColumn + scaled(PANES_GAP);
		m_detailWidth = m_contentWidth - thumbColumn - scaled(PANES_GAP);
	}

	void TitleView::getThumbRect(int index, int& outX, int& outY, int& outWidth, int& outHeight) const
	{
		outX = m_contentX;
		outWidth = scaled(THUMB_COLUMN_WIDTH);
		outHeight = scaled(THUMB_HEIGHT);
		outY = m_panesTop + index * (outHeight + scaled(THUMB_GAP));
	}

	void TitleView::getNavSettingsRect(int& outX, int& outY, int& outWidth, int& outHeight) const
	{
		// 実物と同じく左下の隅に置く
		outX = m_windowX + scaled(NAV_LEFT_MARGIN);
		outWidth = m_navWidth - scaled(NAV_RIGHT_MARGIN);
		outHeight = scaled(NAV_ITEM_HEIGHT);
		outY = m_windowY + m_windowHeight - scaled(NAV_BOTTOM_GAP) - outHeight;
	}

	void TitleView::getExitButtonRect(int& outX, int& outY, int& outWidth, int& outHeight) const
	{
		// 実物が操作ボタンを置く、見出しと同じ高さの右端
		outWidth = scaled(EXIT_BUTTON_WIDTH);
		outHeight = scaled(BUTTON_HEIGHT);
		outX = m_contentX + m_contentWidth - outWidth;
		outY = m_contentY + (scaled(APP_ICON_SIZE) - outHeight) / 2;
	}

	void TitleView::getStartButtonRect(int& outX, int& outY, int& outWidth, int& outHeight) const
	{
		outWidth = scaled(START_BUTTON_WIDTH);
		outHeight = scaled(START_BUTTON_HEIGHT);
		outX = m_contentX + m_contentWidth - outWidth;
		outY = m_contentY + m_contentHeight - outHeight;
	}

	TitleView::Hit TitleView::getHitAt(int x, int y) const
	{
		const auto contains{ [x, y](int rx, int ry, int rw, int rh)
			{ return x >= rx && x < rx + rw && y >= ry && y < ry + rh; } };

		int rectX{}, rectY{}, rectWidth{}, rectHeight{};

		getStartButtonRect(rectX, rectY, rectWidth, rectHeight);
		if (contains(rectX, rectY, rectWidth, rectHeight))
			return Hit::Start;

		getExitButtonRect(rectX, rectY, rectWidth, rectHeight);
		if (contains(rectX, rectY, rectWidth, rectHeight))
			return Hit::Exit;

		getNavSettingsRect(rectX, rectY, rectWidth, rectHeight);
		if (contains(rectX, rectY, rectWidth, rectHeight))
			return Hit::Settings;

		constexpr Hit THUMB_HITS[]{ Hit::ThumbCpu, Hit::ThumbMemory, Hit::ThumbDisk };
		for (int i{ 0 }; i < CHANNEL_COUNT; ++i)
		{
			getThumbRect(i, rectX, rectY, rectWidth, rectHeight);
			if (contains(rectX, rectY, rectWidth, rectHeight))
				return THUMB_HITS[i];
		}

		return Hit::None;
	}

	void TitleView::drawTitle() const
	{
		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);

		drawWindow();
		drawTitleBar();
		drawNav();
		drawAppHeader();
		drawExitButton();
		drawThumbnails();
		drawDetail();
		drawStartButton();
		drawFocusRing();
		drawIntroVeil();

		m_uiRenderer.resetFont();
	}

	void TitleView::drawIntroVeil() const
	{
		const float progress{ introProgress(0.0f, INTRO_VEIL_DURATION) };
		if (progress >= 1.0f)
			return;

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA,
		    static_cast<int>(255.0f * (1.0f - progress)));
		m_uiRenderer.drawBox(0, 0, m_screen.getWidth(), m_screen.getHeight(),
		    Color::TITLE_WINDOW_BG, true);
		m_uiRenderer.resetBlendMode();
	}

	void TitleView::drawWindow() const
	{
		m_uiRenderer.drawBox(m_windowX, m_windowY, m_windowWidth, m_windowHeight,
		    Color::TITLE_WINDOW_BG, true);

		// 右のコンテンツ面。最大化したウィンドウで丸いのは左上だけなので、
		// 右端と下端を画面の外へはみ出させ、そちら側の角丸を切り落とす
		const int radius{ scaled(CONTENT_RADIUS) };
		m_uiRenderer.drawRoundedBox(m_windowX + m_navWidth, m_windowY + m_titleBarHeight,
		    m_windowWidth - m_navWidth + radius, m_windowHeight - m_titleBarHeight + radius,
		    radius, Color::TITLE_CONTENT_BG, true, 1);
	}

	void TitleView::drawTitleBar() const
	{
		const int fontSize{ scaled(FONT_TITLE_BAR) };
		const std::string title{ toDrawable("タスク マネージャー") };

		m_uiRenderer.drawText(m_windowX + scaled(CONTENT_PADDING_X),
		    m_windowY + (m_titleBarHeight - fontSize) / 2,
		    title.c_str(), Color::TITLE_TEXT, fontSize);
	}

	void TitleView::drawNav() const
	{
		const int fontSize{ scaled(FONT_NAV) };
		const int itemX{ m_windowX + scaled(NAV_LEFT_MARGIN) };
		const int itemWidth{ m_navWidth - scaled(NAV_RIGHT_MARGIN) };
		const int itemHeight{ scaled(NAV_ITEM_HEIGHT) };

		// 「パフォーマンス」はいま出している内容そのものなので、選択中として出す。
		// 押しても行き先が変わらないため、押せる場所にはしない
		const int performanceY{ m_windowY + m_titleBarHeight + scaled(NAV_TOP_GAP) };
		m_uiRenderer.drawRoundedBox(itemX, performanceY, itemWidth, itemHeight,
		    scaled(5.0f), Color::TITLE_NAV_SELECTED, true, 1);

		const int pillHeight{ scaled(NAV_PILL_HEIGHT) };
		m_uiRenderer.drawRoundedBox(itemX, performanceY + (itemHeight - pillHeight) / 2,
		    scaled(NAV_PILL_WIDTH), pillHeight, scaled(2.0f), Color::TITLE_ACCENT, true, 1);

		const std::string performance{ toDrawable("パフォーマンス") };
		m_uiRenderer.drawText(itemX + scaled(NAV_TEXT_INDENT),
		    performanceY + (itemHeight - fontSize) / 2,
		    performance.c_str(), Color::TITLE_TEXT, fontSize);

		// 左下の「設定」。実物もアプリの設定をここへ置く
		int settingsX{}, settingsY{}, settingsWidth{}, settingsHeight{};
		getNavSettingsRect(settingsX, settingsY, settingsWidth, settingsHeight);

		if (isHighlighted(Hit::Settings))
		{
			m_uiRenderer.drawRoundedBox(settingsX, settingsY, settingsWidth, settingsHeight,
			    scaled(5.0f), Color::TITLE_CARD_HOVER, true, 1);
		}

		const int iconSize{ scaled(NAV_ICON_SIZE) };
		drawGearIcon(settingsX + scaled(NAV_ICON_LEFT) + iconSize / 2,
		    settingsY + settingsHeight / 2, iconSize, Color::TITLE_TEXT);

		const std::string settings{ toDrawable("設定") };
		m_uiRenderer.drawText(settingsX + scaled(NAV_TEXT_INDENT),
		    settingsY + (settingsHeight - fontSize) / 2,
		    settings.c_str(), Color::TITLE_TEXT, fontSize);
	}

	void TitleView::drawAppHeader() const
	{
		const int alpha{ introAlpha(INTRO_HEADER_START, INTRO_HEADER_DURATION) };
		if (alpha <= 0)
			return;

		m_uiRenderer.setDrawOffset(0, introSlide(INTRO_HEADER_START, INTRO_HEADER_DURATION));
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);

		const int iconSize{ scaled(APP_ICON_SIZE) };

		if (m_iconHandle != -1)
			m_uiRenderer.drawImage(m_iconHandle, m_contentX, m_contentY, iconSize, iconSize);

		const int textX{ m_contentX + iconSize + scaled(APP_ICON_GAP) };
		const int titleFontSize{ scaled(FONT_GAME_TITLE) };
		const int subFontSize{ scaled(FONT_APP_SUB) };

		// 実物は「パフォーマンス」と出す位置。画面で最初に目が行く場所なのでゲーム名を置く
		m_uiRenderer.drawText(textX, m_contentY, "Win vs Mac", Color::TITLE_TEXT, titleFontSize);

		const std::string sub{ toDrawable("WinVsMac.exe ・ 実行中") };
		m_uiRenderer.drawText(textX, m_contentY + titleFontSize + scaled(4.0f),
		    sub.c_str(), Color::TITLE_TEXT_TERTIARY, subFontSize);

		m_uiRenderer.resetBlendMode();
		m_uiRenderer.resetDrawOffset();
	}

	void TitleView::drawExitButton() const
	{
		const int alpha{ introAlpha(INTRO_BUTTON_START, INTRO_BUTTON_DURATION) };
		if (alpha <= 0)
			return;

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);

		int rectX{}, rectY{}, rectWidth{}, rectHeight{};
		getExitButtonRect(rectX, rectY, rectWidth, rectHeight);

		// 実物は「タスクを終了する」だが、それだと何が終わるのか伝わらない。
		// ここは世界観より、押した先が分かることを優先する
		drawButton(rectX, rectY, rectWidth, rectHeight, "ゲームを終了する", false, isHighlighted(Hit::Exit));

		m_uiRenderer.resetBlendMode();
	}

	void TitleView::drawThumbnails() const
	{
		const int alpha{ introAlpha(INTRO_PANEL_START, INTRO_PANEL_DURATION) };
		if (alpha <= 0)
			return;

		m_uiRenderer.setDrawOffset(0, introSlide(INTRO_PANEL_START, INTRO_PANEL_DURATION));
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);

		const int nameFontSize{ scaled(FONT_THUMB_NAME) };
		const int valueFontSize{ scaled(FONT_THUMB_VALUE) };
		const int graphWidth{ scaled(THUMB_GRAPH_WIDTH) };
		const int graphHeight{ scaled(THUMB_GRAPH_HEIGHT) };
		const int padding{ scaled(THUMB_PADDING) };

		constexpr Hit THUMB_HITS[]{ Hit::ThumbCpu, Hit::ThumbMemory, Hit::ThumbDisk };

		for (int i{ 0 }; i < CHANNEL_COUNT; ++i)
		{
			int rectX{}, rectY{}, rectWidth{}, rectHeight{};
			getThumbRect(i, rectX, rectY, rectWidth, rectHeight);

			const bool isSelected{ i == m_selectedChannel };
			if (isSelected || isHighlighted(THUMB_HITS[i]))
			{
				m_uiRenderer.drawRoundedBox(rectX, rectY, rectWidth, rectHeight, scaled(4.0f),
				    isSelected ? Color::TITLE_NAV_SELECTED : Color::TITLE_CARD_HOVER, true, 1);
			}

			const int graphX{ rectX + padding };
			const int graphY{ rectY + (rectHeight - graphHeight) / 2 };
			m_uiRenderer.drawBox(graphX, graphY, graphWidth, graphHeight, Color::TITLE_CARD, true);
			drawGraph(graphX, graphY, graphWidth, graphHeight, i, false, alpha);
			m_uiRenderer.drawBox(graphX, graphY, graphWidth, graphHeight, Color::TITLE_STROKE, false);

			const int textX{ graphX + graphWidth + scaled(THUMB_TEXT_GAP) };
			const std::string name{ toDrawable(CHANNEL_SPECS[i].m_name) };
			m_uiRenderer.drawText(textX, graphY + scaled(8.0f), name.c_str(), Color::TITLE_TEXT, nameFontSize);

			const std::string value{ std::to_string(
				                         static_cast<int>(m_channels[i].m_history.back() * 100.0f)) +
				                     "%" };
			m_uiRenderer.drawText(textX, graphY + scaled(8.0f) + nameFontSize + scaled(4.0f),
			    value.c_str(), Color::TITLE_TEXT_TERTIARY, valueFontSize);
		}

		m_uiRenderer.resetBlendMode();
		m_uiRenderer.resetDrawOffset();
	}

	void TitleView::drawDetail() const
	{
		const int alpha{ introAlpha(INTRO_PANEL_START, INTRO_PANEL_DURATION) };
		if (alpha <= 0)
			return;

		m_uiRenderer.setDrawOffset(0, introSlide(INTRO_PANEL_START, INTRO_PANEL_DURATION));
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);

		const ChannelSpec& spec{ CHANNEL_SPECS[m_selectedChannel] };

		// 実物が「ディスク 0 (C:) ／ 型番」を出す位置。こちらはチャンネル名と取得元を出す
		const int nameFontSize{ scaled(FONT_CHANNEL_NAME) };
		const std::string channelName{ toDrawable(spec.m_name) };
		m_uiRenderer.drawText(m_detailX, m_panesTop, channelName.c_str(), Color::TITLE_TEXT, nameFontSize);

		const int smallFontSize{ scaled(FONT_SMALL) };
		const std::string source{ toDrawable("このPCの実測値") };
		const int sourceWidth{ m_uiRenderer.getTextWidth(source.c_str(), smallFontSize) };
		m_uiRenderer.drawText(m_detailX + m_detailWidth - sourceWidth,
		    m_panesTop + (nameFontSize - smallFontSize) / 2,
		    source.c_str(), Color::TITLE_TEXT_TERTIARY, smallFontSize);

		// グラフの上下に付く小さな説明（実物と同じ位置・同じ内容）
		const int captionY{ m_panesTop + scaled(DETAIL_HEAD_HEIGHT) };
		const std::string caption{ toDrawable(spec.m_caption) };
		m_uiRenderer.drawText(m_detailX, captionY, caption.c_str(), Color::TITLE_TEXT_TERTIARY, smallFontSize);

		const int maxWidth{ m_uiRenderer.getTextWidth("100%", smallFontSize) };
		m_uiRenderer.drawText(m_detailX + m_detailWidth - maxWidth, captionY,
		    "100%", Color::TITLE_TEXT_TERTIARY, smallFontSize);

		// グラフ本体。残りの高さをすべて使う
		const int graphY{ captionY + scaled(GRAPH_CAPTION_HEIGHT) };
		const int statsY{ m_contentY + m_contentHeight - scaled(START_BUTTON_HEIGHT) - scaled(START_BUTTON_TOP_GAP) - scaled(STATS_HEIGHT) };
		const int graphHeight{ statsY - scaled(STATS_TOP_GAP) - scaled(GRAPH_AXIS_HEIGHT) - graphY };

		m_uiRenderer.drawBox(m_detailX, graphY, m_detailWidth, graphHeight, Color::TITLE_CARD, true);
		drawGraph(m_detailX, graphY, m_detailWidth, graphHeight, m_selectedChannel, true, alpha);
		m_uiRenderer.drawBox(m_detailX, graphY, m_detailWidth, graphHeight, Color::TITLE_STROKE, false);

		const int axisY{ graphY + graphHeight + scaled(4.0f) };
		const std::string span{ toDrawable("60 秒") };
		m_uiRenderer.drawText(m_detailX, axisY, span.c_str(), Color::TITLE_TEXT_TERTIARY, smallFontSize);
		const int zeroWidth{ m_uiRenderer.getTextWidth("0", smallFontSize) };
		m_uiRenderer.drawText(m_detailX + m_detailWidth - zeroWidth, axisY,
		    "0", Color::TITLE_TEXT_TERTIARY, smallFontSize);

		drawStats(statsY);

		m_uiRenderer.resetBlendMode();
		m_uiRenderer.resetDrawOffset();
	}

	void TitleView::drawGraph(int x, int y, int width, int height, int channelIndex,
	    bool withGrid, int groupAlpha) const
	{
		const unsigned int color{ CHANNEL_SPECS[channelIndex].m_color };
		const std::array<float, HISTORY_SIZE>& history{ m_channels[channelIndex].m_history };

		if (withGrid)
		{
			// 実物と同じ細かい方眼。白地に薄いグレーを置くだけなので合成は要らない
			for (int i{ 1 }; i < static_cast<int>(GRID_LINE_COUNT); ++i)
			{
				const int lineY{ y + static_cast<int>(height * i / GRID_LINE_COUNT) };
				m_uiRenderer.drawBox(x, lineY, width, 1, Color::TITLE_GRID, true);

				const int lineX{ x + static_cast<int>(width * i / GRID_LINE_COUNT) };
				m_uiRenderer.drawBox(lineX, y, 1, height, Color::TITLE_GRID, true);
			}
		}

		for (int i{ 0 }; i < HISTORY_SIZE; ++i)
		{
			const int barHeight{ static_cast<int>(history[i] * height) };
			if (barHeight <= 0)
				continue;

			// 幅は次の棒の位置から決める。固定幅だと割り切れないぶんの隙間が縞になって出る
			const int barX{ x + i * width / HISTORY_SIZE };
			const int barWidth{ std::max(1, x + (i + 1) * width / HISTORY_SIZE - barX) };
			const int barY{ y + height - barHeight };

			// 塗り（薄く）と上端の線。実物の面グラフに近い見え方になる
			m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA,
			    GRAPH_FILL_ALPHA * groupAlpha / 255);
			m_uiRenderer.drawBox(barX, barY, barWidth, barHeight, color, true);

			// 上端は白地に対してそのまま置く。薄めると塗りとの差が出ず輪郭が消える。
			// 呼び出し側の濃さへ戻すので、ここを抜けたあとも同じ濃さで描き続けられる
			m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, groupAlpha);
			m_uiRenderer.drawBox(barX, barY, barWidth, std::max(1, scaled(1.6f)), color, true);
		}
	}

	void TitleView::drawStats(int y) const
	{
		const ChannelSpec& spec{ CHANNEL_SPECS[m_selectedChannel] };
		const std::array<float, HISTORY_SIZE>& history{ m_channels[m_selectedChannel].m_history };

		float total{ 0.0f };
		float peak{ 0.0f };
		for (const float value : history)
		{
			total += value;
			peak = std::max(peak, value);
		}

		const int labelFontSize{ scaled(FONT_STAT_LABEL) };
		const int valueFontSize{ scaled(FONT_STAT_VALUE) };
		const int columnWidth{ scaled(STATS_COLUMN_WIDTH) };

		const std::string labels[]{ toDrawable(spec.m_statLabel), toDrawable("平均"), toDrawable("最大") };
		const int values[]{
			static_cast<int>(history.back() * 100.0f),
			static_cast<int>(total / HISTORY_SIZE * 100.0f),
			static_cast<int>(peak * 100.0f),
		};

		for (int i{ 0 }; i < 3; ++i)
		{
			const int columnX{ m_detailX + i * columnWidth };
			m_uiRenderer.drawText(columnX, y, labels[i].c_str(), Color::TITLE_TEXT_TERTIARY, labelFontSize);

			const std::string text{ std::to_string(values[i]) + "%" };
			m_uiRenderer.drawText(columnX, y + labelFontSize + scaled(4.0f),
			    text.c_str(), Color::TITLE_TEXT, valueFontSize);
		}
	}

	void TitleView::drawStartButton() const
	{
		const int alpha{ introAlpha(INTRO_BUTTON_START, INTRO_BUTTON_DURATION) };
		if (alpha <= 0)
			return;

		int rectX{}, rectY{}, rectWidth{}, rectHeight{};
		getStartButtonRect(rectX, rectY, rectWidth, rectHeight);

		// カーソルが乗っているなら気づけているので、呼び込みは出さない
		if (!isHighlighted(Hit::Start))
			drawStartPulse(rectX, rectY, rectWidth, rectHeight);

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);
		drawButton(rectX, rectY, rectWidth, rectHeight, "選択画面へ", true, isHighlighted(Hit::Start));
		m_uiRenderer.resetBlendMode();
	}

	void TitleView::drawStartPulse(int x, int y, int width, int height) const
	{
		// 起動演出の途中はまだボタンが出そろっていないので、呼び込みは始めない
		if (!isIntroFinished() || m_pulseTimer >= PULSE_DURATION)
			return;

		const float progress{ m_pulseTimer / PULSE_DURATION };

		// 勢いよく広がってから緩める。等速だと輪が伸びていくようにしか見えない
		const int spread{ static_cast<int>(scaled(PULSE_MAX_SPREAD) * core::utility::easeOut(progress)) };
		const int alpha{ static_cast<int>(PULSE_MAX_ALPHA * (1.0f - progress)) };
		if (alpha <= 0)
			return;

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);
		m_uiRenderer.drawRoundedBox(x - spread, y - spread, width + spread * 2, height + spread * 2,
		    scaled(BUTTON_RADIUS) + spread, Color::TITLE_ACCENT, false, scaled(PULSE_THICKNESS));
		m_uiRenderer.resetBlendMode();
	}

	void TitleView::drawGearIcon(int centerX, int centerY, int size, unsigned int color) const
	{
		const float ringRadius{ size * GEAR_RING_RADIUS_RATIO };
		const int thickness{ std::max(1, static_cast<int>(size * GEAR_RING_THICKNESS_RATIO)) };
		const float toothLength{ size * GEAR_TOOTH_LENGTH_RATIO };

		// 歯は輪の内側から生やす。輪の中心線から出すと、根元に隙間ができて割れて見える
		const float toothStart{ ringRadius - thickness * 0.5f };

		for (int i{ 0 }; i < GEAR_TOOTH_COUNT; ++i)
		{
			const float angle{ core::utility::TWO_PI * i / GEAR_TOOTH_COUNT };
			const float dirX{ std::cos(angle) };
			const float dirY{ std::sin(angle) };

			m_uiRenderer.drawLine(centerX + static_cast<int>(dirX * toothStart),
			    centerY + static_cast<int>(dirY * toothStart),
			    centerX + static_cast<int>(dirX * toothLength),
			    centerY + static_cast<int>(dirY * toothLength),
			    color, thickness);
		}

		// 塗らずに描くことで、中央が抜けて歯車の穴になる
		m_uiRenderer.drawCircle(centerX, centerY, static_cast<int>(ringRadius), color, false, thickness);
	}

	void TitleView::drawButton(int x, int y, int width, int height, const char* label,
	    bool isAccent, bool isHovered) const
	{
		const int radius{ scaled(BUTTON_RADIUS) };

		if (isAccent)
		{
			m_uiRenderer.drawRoundedBox(x, y, width, height, radius, Color::TITLE_ACCENT, true, 1);
		}
		else
		{
			// 白い面の上に白いボタンを置くので、枠は常に描く。
			// 乗せたときだけ枠を出す作りだと、押せる場所があること自体が伝わらない
			m_uiRenderer.drawRoundedBox(x, y, width, height, radius,
			    isHovered ? Color::TITLE_CARD_HOVER : Color::TITLE_CARD, true, 1);
			m_uiRenderer.drawRoundedBox(x, y, width, height, radius, Color::TITLE_STROKE, false, 1);
		}

		// アクセントのボタンは押した先が分かるよう、乗せたときだけ少し暗くする
		if (isAccent && isHovered)
		{
			m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, 40);
			m_uiRenderer.drawRoundedBox(x, y, width, height, radius, Color::BLACK, true, 1);
			m_uiRenderer.resetBlendMode();
		}

		const int fontSize{ scaled(isAccent ? FONT_START_BUTTON : FONT_BUTTON) };
		const std::string text{ toDrawable(label) };
		const int textWidth{ m_uiRenderer.getTextWidth(text.c_str(), fontSize) };

		m_uiRenderer.drawText(x + (width - textWidth) / 2, y + (height - fontSize) / 2,
		    text.c_str(), isAccent ? Color::WHITE : Color::TITLE_TEXT, fontSize);
	}

	std::string TitleView::toDrawable(const char* utf8) const
	{
		// DxLibのマルチバイト描画に合わせてUTF-8からShift-JISへ変換する
		auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };
		if (!converter)
			return std::string{ utf8 };

		return converter->utf8ToShiftJis(utf8);
	}
} // namespace game::scene
