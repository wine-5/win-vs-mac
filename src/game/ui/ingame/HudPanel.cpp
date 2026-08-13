#include "HudPanel.h"
#include "core/constant/UI.h"
#include "core/utility/Color.h"
#include "core/utility/MathConstants.h"
#include <cmath>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	constexpr int PANEL_RADIUS{ 8 }; // Windows 11のウィンドウ・パネルの角丸

	// 塗りと枠。DxLibのブレンドはアルファ値を別途指定するため、色と不透明度を分けて持つ
	constexpr unsigned int PANEL_FILL_COLOR{ core::utility::Color::HUD_PANEL_FILL };
	constexpr int PANEL_FILL_ALPHA{ 184 }; // 約72%
	constexpr unsigned int PANEL_BORDER_COLOR{ core::utility::Color::HUD_PANEL_BORDER };
	constexpr int PANEL_BORDER_ALPHA{ 46 }; // 約18%

	// 面を横切る光の帯
	constexpr float SWEEP_INTERVAL{ 6.0f }; // 光り始めから次に光り始めるまで（秒）
	constexpr float SWEEP_DURATION{ 0.9f }; // 帯が左端から右端まで抜けるのにかかる時間（秒）
	constexpr int SWEEP_BAND_WIDTH{ 110 };  // 帯の幅（1080p基準）
	constexpr int SWEEP_SLICE_COUNT{ 20 };  // 帯を何枚の短冊に分けて濃淡を付けるか
	constexpr int SWEEP_ALPHA{ 26 };        // 短冊1枚あたりの明るさ（重なって帯になる）
} // namespace

namespace game::ui::ingame
{
	HudPanel::HudPanel(core::iface::IUIRenderer& uiRenderer, core::iface::IScreen& screen)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	{
	}

	void HudPanel::draw(int x, int y, int width, int height, bool withSweep)
	{
		const int radius{ PANEL_RADIUS * m_screen.getHeight() / BASE_SCREEN_HEIGHT };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, PANEL_FILL_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, PANEL_FILL_COLOR, true, 1);

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, PANEL_BORDER_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, PANEL_BORDER_COLOR, false, 1);

		m_uiRenderer.resetBlendMode();

		if (withSweep)
			drawSweep(x, y, width, height);
	}

	void HudPanel::drawSweep(int x, int y, int width, int height)
	{
		const float elapsed{ std::chrono::duration<float>(
			std::chrono::steady_clock::now() - m_startTime)
			    .count() };

		// 周期の頭のSWEEP_DURATIONぶんだけ走らせ、残りは何も描かずに間を空ける
		const float phase{ std::fmod(elapsed, SWEEP_INTERVAL) };
		if (phase > SWEEP_DURATION)
			return;

		const int scale{ m_screen.getHeight() / BASE_SCREEN_HEIGHT };
		const int bandWidth{ SWEEP_BAND_WIDTH * (scale > 0 ? scale : 1) };
		const float progress{ phase / SWEEP_DURATION };

		// 帯の中心が左端の外から右端の外まで通り抜ける
		const int centerX{ x - bandWidth + static_cast<int>((width + bandWidth * 2) * progress) };

		// 角丸の外へはみ出さないよう、上下を角丸の半径ぶん詰めた範囲にだけ描く。
		// DxLibに矩形以外のクリップが無いため、形状側を避ける
		const int radius{ PANEL_RADIUS * (scale > 0 ? scale : 1) };
		const int bandTop{ y + radius };
		const int bandHeight{ height - radius * 2 };
		if (bandHeight <= 0)
			return;

		// グラデーションが無いので、短冊を並べて中央ほど明るくする。
		// 加算合成なので重なるほど明るくなり、境目が目立たない
		const int sliceWidth{ bandWidth / SWEEP_SLICE_COUNT };
		if (sliceWidth <= 0)
			return;

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ADD, SWEEP_ALPHA);
		for (int i{ 0 }; i < SWEEP_SLICE_COUNT; ++i)
		{
			// 帯の中央が最も明るく、両端で0になる山なりの分布
			const float t{ (i + 0.5f) / SWEEP_SLICE_COUNT };
			const int sliceX{ centerX - bandWidth / 2 + i * sliceWidth };

			// パネルの外へ出た短冊は描かない（隣のパネルや3D画面まで光ってしまうため）
			if (sliceX + sliceWidth <= x || sliceX >= x + width)
				continue;

			const int clippedLeft{ sliceX < x ? x : sliceX };
			const int clippedRight{ sliceX + sliceWidth > x + width ? x + width : sliceX + sliceWidth };

			// 山の形はsinで作る。明るさは短冊を重ねる回数で表現する
			const int repeat{ static_cast<int>(std::sin(t * core::utility::PI) * 3.0f) };
			for (int n{ 0 }; n < repeat; ++n)
				m_uiRenderer.drawBox(clippedLeft, bandTop, clippedRight - clippedLeft, bandHeight,
				    core::utility::Color::WHITE, true);
		}
		m_uiRenderer.resetBlendMode();
	}
} // namespace game::ui::ingame
