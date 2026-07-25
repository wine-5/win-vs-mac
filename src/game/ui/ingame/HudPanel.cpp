#include "HudPanel.h"
#include "core/constant/UI.h"
#include "core/utility/Color.h"

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	constexpr int PANEL_RADIUS{ 8 }; // Windows 11のウィンドウ・パネルの角丸

	// 塗りと枠。DxLibのブレンドはアルファ値を別途指定するため、色と不透明度を分けて持つ
	constexpr unsigned int PANEL_FILL_COLOR{ 0xFF0E1420 };
	constexpr int PANEL_FILL_ALPHA{ 184 }; // 約72%
	constexpr unsigned int PANEL_BORDER_COLOR{ 0xFF8CAAD2 };
	constexpr int PANEL_BORDER_ALPHA{ 46 }; // 約18%
} // namespace

namespace game::ui::ingame
{
	HudPanel::HudPanel(core::iface::IUIRenderer& uiRenderer, core::iface::IScreen& screen)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	{
	}

	void HudPanel::draw(int x, int y, int width, int height)
	{
		const int radius{ PANEL_RADIUS * m_screen.getHeight() / BASE_SCREEN_HEIGHT };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, PANEL_FILL_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, PANEL_FILL_COLOR, true, 1);

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, PANEL_BORDER_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, width, height, radius, PANEL_BORDER_COLOR, false, 1);

		m_uiRenderer.resetBlendMode();
	}
} // namespace game::ui::ingame
