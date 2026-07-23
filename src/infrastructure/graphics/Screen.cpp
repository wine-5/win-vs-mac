#include "Screen.h"
#include "DxLib.h"

namespace infrastructure::graphics
{
	Screen::Screen(int width, int height)
		: m_width{width}
		, m_height{height}
	{
	}

	int Screen::getWidth() const noexcept
	{
		return m_width;
	}

	int Screen::getHeight() const noexcept
	{
		return m_height;
	}

	void* Screen::getNativeWindowHandle() const noexcept
	{
		return GetMainWindowHandle();
	}

	void Screen::setBackgroundColor(int r, int g, int b) noexcept
	{
		// ClearDrawScreen で塗られる背景色を変更する
		SetBackgroundColor(r, g, b);
	}

} // namespace infrastructure::graphics