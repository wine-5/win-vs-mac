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

	void Screen::setFog(bool enable, int r, int g, int b, float startDistance, float endDistance) noexcept
	{
		SetFogEnable(enable ? TRUE : FALSE);
		if (!enable)
			return;

		SetFogColor(r, g, b);
		// ヘッダのコメントは 0.0〜1.0 とあるが、実際はワールド距離で指定する
		// （0.15/0.60 を渡すと全面がフォグ色に潰れることを実機で確認済み）
		SetFogStartEnd(startDistance, endDistance);
	}

} // namespace infrastructure::graphics