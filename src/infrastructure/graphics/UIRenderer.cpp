#include "UIRenderer.h"
#include "DxLib.h"
#include <cstring>

namespace infrastructure::graphics
{
	UIRenderer::UIRenderer(std::string defaultFontName)
		: m_defaultFontName{std::move(defaultFontName)}, m_currentFontName{m_defaultFontName}
	{
	}

	UIRenderer::~UIRenderer()
	{
		for (auto &[key, handle] : m_fontHandles)
			DeleteFontToHandle(handle);
	}

	void UIRenderer::drawBox(int x, int y, int width, int height, unsigned int color, bool isFilled)
	{
		x += m_offsetX;
		y += m_offsetY;
		if (isFilled)
			DrawBox(x, y, x + width, y + height, color, TRUE);
		else
			DrawBox(x, y, x + width, y + height, color, FALSE);
	}

	void UIRenderer::drawCircle(int centerX, int centerY, int radius, unsigned int color, bool isFilled, int thickness)
	{
		DrawCircle(centerX + m_offsetX, centerY + m_offsetY, radius, color, isFilled ? TRUE : FALSE, thickness);
	}

	void UIRenderer::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, unsigned int color, bool isFilled)
	{
		DrawTriangle(x1 + m_offsetX, y1 + m_offsetY, x2 + m_offsetX, y2 + m_offsetY,
		    x3 + m_offsetX, y3 + m_offsetY, color, isFilled ? TRUE : FALSE);
	}

	void UIRenderer::drawLine(int x1, int y1, int x2, int y2, unsigned int color, int thickness)
	{
		DrawLineAA(static_cast<float>(x1 + m_offsetX), static_cast<float>(y1 + m_offsetY),
		    static_cast<float>(x2 + m_offsetX), static_cast<float>(y2 + m_offsetY),
		    color, static_cast<float>(thickness));
	}

	void UIRenderer::drawRoundedBox(int x, int y, int width, int height, int radius, unsigned int color, bool isFilled, int thickness)
	{
		// 角の円弧を何分割して描くか。Windows 11の角丸（4〜8px）ならこの程度で十分滑らかになる
		constexpr int CORNER_SEGMENTS{ 12 };

		x += m_offsetX;
		y += m_offsetY;
		DrawRoundRectAA(static_cast<float>(x), static_cast<float>(y),
		    static_cast<float>(x + width), static_cast<float>(y + height),
		    static_cast<float>(radius), static_cast<float>(radius),
		    CORNER_SEGMENTS, color, isFilled ? TRUE : FALSE, static_cast<float>(thickness));
	}

	int UIRenderer::resolveFontHandle(int fontSize) const
	{
		const auto key{ std::make_pair(m_currentFontName, fontSize) };
		auto it{ m_fontHandles.find(key) };
		if (it != m_fontHandles.end())
			return it->second;

		const char* fontName{ m_currentFontName.empty() ? nullptr : m_currentFontName.c_str() };
		// フォントハンドルは1つあたり約4MBを確保する。サイズ違いは別ハンドルになるため、
		// 拡縮アニメで1pxずつサイズを変えると種類が際限なく増える。呼び出し側でサイズを丸めること
		const int handle{ CreateFontToHandle(fontName, fontSize, -1, DX_FONTTYPE_NORMAL) };
		m_fontHandles[key] = handle;
		return handle;
	}

	void UIRenderer::drawText(int x, int y, const char *text, unsigned int color, int fontSize)
	{
		DrawStringToHandle(x + m_offsetX, y + m_offsetY, text, color, resolveFontHandle(fontSize));
	}

	int UIRenderer::getTextWidth(const char *text, int fontSize) const
	{
		return GetDrawStringWidthToHandle(text, static_cast<int>(std::strlen(text)),
		    resolveFontHandle(fontSize));
	}

	void UIRenderer::setBlendMode(int blendMode, int alpha)
	{
		SetDrawBlendMode(blendMode, alpha);
	}

	void UIRenderer::resetBlendMode()
	{
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
	}

	void UIRenderer::setFont(const char *fontName)
	{
		m_currentFontName = fontName;
	}

	void UIRenderer::resetFont()
	{
		m_currentFontName = m_defaultFontName;
	}

	void UIRenderer::drawImage(int handle, int x, int y, int width, int height)
	{
		if (handle == -1) return;
		x += m_offsetX;
		y += m_offsetY;
		// 第6引数は透過フラグ。PNGのアルファチャンネルを反映するためTRUEにする
		// （アルファを持たないJPG等は不透明のままなので影響なし）
		DrawExtendGraph(x, y, x + width, y + height, handle, TRUE);
	}

	void UIRenderer::setClipArea(int x, int y, int width, int height)
	{
		// 切り抜き範囲もずらす。ずらさないとミニマップだけ中身が動いて枠が残る
		x += m_offsetX;
		y += m_offsetY;
		// DxLibの描画可能範囲は右下端を含むため、幅・高さから1引いた座標を渡す
		SetDrawArea(x, y, x + width - 1, y + height - 1);
	}

	void UIRenderer::resetClipArea()
	{
		int screenWidth{ 0 };
		int screenHeight{ 0 };
		int colorBitDepth{ 0 };
		GetScreenState(&screenWidth, &screenHeight, &colorBitDepth);
		SetDrawArea(0, 0, screenWidth, screenHeight);
	}

	void UIRenderer::setDrawOffset(int x, int y)
	{
		m_offsetX = x;
		m_offsetY = y;
	}

	void UIRenderer::resetDrawOffset()
	{
		m_offsetX = 0;
		m_offsetY = 0;
	}
} // namespace infrastructure::graphics