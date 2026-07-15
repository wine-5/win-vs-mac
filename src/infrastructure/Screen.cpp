#include "Screen.h"
#include "DxLib.h"

namespace infrastructure
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

} // namespace infrastructure