#include "Screen.h"
#include <DxLib.h>

namespace infrastructure
{
	Screen::Screen()
		: m_width{}
		, m_height{}
	{
		// DxLibから現在の画面サイズを取得する
		GetWindowSize(&m_width, &m_height);
	}

	int Screen::getWidth() const noexcept
	{
		return m_width;
	}

	int Screen::getHeight() const noexcept
	{
		return m_height;
	}

}