#include "TitleView.h"
#include "game/ui/Button.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IStringConverter.h"
#include <string>
#include <algorithm>

namespace game::scene
{
	TitleView::TitleView(core::iface::IInputProvider& inputProvider,
		core::iface::IUIRenderer& uiRenderer,
		core::iface::IScreen& screen,
		std::string mainFontName,
		std::function<void()> onGoToSelect,
		std::function<void()> onExit)
		: m_uiRenderer{ uiRenderer }
		, m_screen{ screen }
		, m_mainFontName{ std::move(mainFontName) }
	{
		const int screenWidth    { screen.getWidth() };
		const int screenHeight   { screen.getHeight() };
		const int buttonWidth    { static_cast<int>(screenWidth  * BUTTON_WIDTH_RATIO) };
		const int buttonHeight   { static_cast<int>(screenHeight * BUTTON_HEIGHT_RATIO) };
		const int buttonX        { (screenWidth - buttonWidth) / 2 };
		const int startButtonY   { static_cast<int>(screenHeight * START_BUTTON_Y_RATIO) };
		const int exitButtonY    { static_cast<int>(screenHeight * EXIT_BUTTON_Y_RATIO) };
		const int buttonFontSize { static_cast<int>(screenHeight * core::constant::ui::DEFAULT_FONT_SIZE_RATIO) };

		auto startBtn{ std::make_unique<ui::Button>(
			"選択画面へ", buttonX, startButtonY, buttonWidth, buttonHeight, inputProvider, buttonFontSize) };
		startBtn->setOnClick(std::move(onGoToSelect));
		startBtn->setVisible(false);
		m_startButton = startBtn.get();
		m_uiManager.addElement(std::move(startBtn));

		auto exitBtn{ std::make_unique<ui::Button>(
			"EXEを終了する", buttonX, exitButtonY, buttonWidth, buttonHeight, inputProvider, buttonFontSize) };
		exitBtn->setOnClick(std::move(onExit));
		exitBtn->setVisible(false);
		m_exitButton = exitBtn.get();
		m_uiManager.addElement(std::move(exitBtn));
	}

	void TitleView::update(const core::iface::PerformanceSnapshot& snap)
	{
		// EMA スムージングを通して履歴に積み上げる
		m_cpuSmoothed  += CPU_SMOOTH_FACTOR  * (snap.cpuUsage    - m_cpuSmoothed);
		m_memSmoothed  += MEM_SMOOTH_FACTOR  * (snap.memoryUsage - m_memSmoothed);
		m_diskSmoothed += DISK_SMOOTH_FACTOR * (snap.diskActivity - m_diskSmoothed);

		pushHistory(m_cpuHistory,  m_cpuSmoothed);
		pushHistory(m_memHistory,  m_memSmoothed);
		pushHistory(m_diskHistory, m_diskSmoothed);

		m_uiManager.update();
	}

	void TitleView::drawTitle() const
	{
		drawBackground();

		m_uiRenderer.setFont(m_mainFontName.c_str());

		const char* title{ "Win vs Mac" };
		const int titleFontSize{ static_cast<int>(m_screen.getHeight() * core::constant::ui::FONT_SIZE_CLOCK_RATIO) };
		const int titleWidth{ m_uiRenderer.getTextWidth(title, titleFontSize) };
		const int titleX{ (m_screen.getWidth() - titleWidth) / 2 };
		const int titleY{ static_cast<int>(m_screen.getHeight() * TITLE_Y_RATIO) };

		m_uiRenderer.drawText(titleX, titleY, title, core::utility::Color::WHITE, titleFontSize);
		m_uiManager.draw(m_uiRenderer);
	}

	void TitleView::setButtonsVisible(bool visible)
	{
		m_startButton->setVisible(visible);
		m_exitButton->setVisible(visible);
	}

	void TitleView::pushHistory(std::array<float, HISTORY_SIZE>& buf, float value)
	{
		std::rotate(buf.begin(), buf.begin() + 1, buf.end());
		buf.back() = value;
	}

	void TitleView::drawBackground() const
	{
		const int screenW{ m_screen.getWidth()  };
		const int screenH{ m_screen.getHeight() };

		struct Channel
		{
			const std::array<float, HISTORY_SIZE>& m_history;
			const char*  m_label;
			unsigned int m_color;
			float        m_topRatio; // カード上端の Y 位置（0〜1）
		};

		const Channel channels[]
		{
			{ m_cpuHistory,  "CPU",    core::utility::Color::GRAPH_CPU,    GRAPH_CPU_TOP_RATIO    },
			{ m_memHistory,  "Memory", core::utility::Color::GRAPH_MEMORY, GRAPH_MEMORY_TOP_RATIO },
			{ m_diskHistory, "Disk",   core::utility::Color::GRAPH_DISK,   GRAPH_DISK_TOP_RATIO   },
		};

		constexpr float CARD_WIDTH_RATIO{ 0.88f };       // 画面幅に対するカード幅の割合
		constexpr float CARD_HEIGHT_RATIO{ 0.17f };      // 画面高さに対するカード高さの割合
		constexpr float GRAPH_WIDTH_FRAC{ 0.68f };       // カード幅に対するグラフ領域の割合
		constexpr float LABEL_FONT_SIZE_RATIO{ 0.028f }; // 画面高さに対するラベルフォントサイズの割合
		constexpr float VALUE_FONT_SIZE_RATIO{ 0.038f }; // 画面高さに対する数値フォントサイズの割合
		constexpr float GRID_LINE_INTERVAL{ 0.25f };     // グリッドライン間隔（0=下端、1=上端）
		constexpr int GRID_LINE_COUNT{ 3 };              // グリッドライン本数（25 / 50 / 75 %）
		constexpr int GRAPH_PADDING{ 2 };                // グラフ領域の上下パディング（px）
		constexpr int INFO_PANEL_PADDING{ 14 };          // 情報パネルの左マージン（px）
		constexpr int TEXT_VERTICAL_OFFSET{ 2 };         // ラベル・数値の縦位置微調整（px）

		const int cardW{ static_cast<int>(screenW * CARD_WIDTH_RATIO) };
		const int cardX { (screenW - cardW) / 2 };
		const int cardH{ static_cast<int>(screenH * CARD_HEIGHT_RATIO) };
		const int graphW{ static_cast<int>(cardW * GRAPH_WIDTH_FRAC) };
		const int infoX{ cardX + graphW + INFO_PANEL_PADDING };
		const int barW  { std::max(1, graphW / HISTORY_SIZE) };

		const int labelSize{ static_cast<int>(screenH * LABEL_FONT_SIZE_RATIO) };
		const int valueSize{ static_cast<int>(screenH * VALUE_FONT_SIZE_RATIO) };

		for (const auto& ch : channels)
		{
			const int cardTop { static_cast<int>(screenH * ch.m_topRatio) };
			const int graphTop{ cardTop + GRAPH_PADDING };
			const int graphBot{ cardTop + cardH - GRAPH_PADDING };
			const int graphH  { graphBot - graphTop };
			const float latestVal{ ch.m_history.back() };

			// カード背景（濃い紺）
			m_uiRenderer.setBlendMode(2, 40);
			m_uiRenderer.drawBox(cardX, cardTop, cardW, cardH,
				core::utility::Color::CARD_BG, true);

			// グラフ背景（チャンネルカラーで極薄）
			m_uiRenderer.setBlendMode(2, 18);
			m_uiRenderer.drawBox(cardX, graphTop, graphW, graphH, ch.m_color, true);

			// 横グリッドライン（25 / 50 / 75 %）
			m_uiRenderer.setBlendMode(2, 40);
			for (int g{ 1 }; g <= GRID_LINE_COUNT; ++g)
			{
				const int gy{ graphBot - static_cast<int>(graphH * g * GRID_LINE_INTERVAL) };
				m_uiRenderer.drawBox(cardX, gy, graphW, 1, ch.m_color, true);
			}

			// 波形（塗り ＋ 上端ライン）
			for (int i{ 0 }; i < HISTORY_SIZE; ++i)
			{
				const int barH{ static_cast<int>(ch.m_history[i] * graphH) };
				if (barH <= 0) continue;
				const int x{ cardX + i * barW };
				const int y{ graphBot - barH };

				// 塗り（半透明）
				m_uiRenderer.setBlendMode(2, 90);
				m_uiRenderer.drawBox(x, y, barW - 1, barH, ch.m_color, true);

				// 上端ライン（輝線）
				m_uiRenderer.setBlendMode(2, 230);
				m_uiRenderer.drawBox(x, y, barW - 1, 1, ch.m_color, true);
			}

			// カード枠線
			m_uiRenderer.setBlendMode(2, 130);
			m_uiRenderer.drawBox(cardX, cardTop, cardW, cardH, ch.m_color, false);

			// グラフ ／ 情報パネル 区切り線
			m_uiRenderer.setBlendMode(2, 100);
			m_uiRenderer.drawBox(cardX + graphW, cardTop, 1, cardH, ch.m_color, true);

			// ラベル（情報パネル上段）
			const int infoCenterY{ cardTop + cardH / 2 };
			m_uiRenderer.setBlendMode(2, 210);
			m_uiRenderer.drawText(infoX, infoCenterY - labelSize - valueSize / 2 - TEXT_VERTICAL_OFFSET,
			                      ch.m_label, ch.m_color, labelSize);

			// 数値（情報パネル下段、白 ＋ 大きめ）
			const std::string valText{ std::to_string(static_cast<int>(latestVal * 100.f)) + "%" };
			m_uiRenderer.setBlendMode(2, 255);
			m_uiRenderer.drawText(infoX, infoCenterY + valueSize / 2 - labelSize,
				valText.c_str(), core::utility::Color::WHITE, valueSize);

			m_uiRenderer.resetBlendMode();
		}
	}
} // namespace game::scene