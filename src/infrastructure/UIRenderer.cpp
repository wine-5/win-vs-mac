#include "UIRenderer.h"
#include "DxLib.h"
#include <cstring>

namespace infrastructure
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
		if (isFilled)
			DrawBox(x, y, x + width, y + height, color, TRUE);
		else
			DrawBox(x, y, x + width, y + height, color, FALSE);
	}

	void UIRenderer::drawText(int x, int y, const char *text, unsigned int color, int fontSize)
	{
		const auto key{std::make_pair(m_currentFontName, fontSize)};
		if (m_fontHandles.find(key) == m_fontHandles.end())
		{
			const char *fontName{m_currentFontName.empty() ? nullptr : m_currentFontName.c_str()};
			m_fontHandles[key] = CreateFontToHandle(fontName, fontSize, -1, DX_FONTTYPE_NORMAL);
		}
		DrawStringToHandle(x, y, text, color, m_fontHandles[key]);
	}

	int UIRenderer::getTextWidth(const char *text, int fontSize) const
	{
		const auto key{ std::make_pair(m_currentFontName, fontSize) };
		auto it{ m_fontHandles.find(key) };
		if (it == m_fontHandles.end())
		{
			const char* fontName{ m_currentFontName.empty() ? nullptr : m_currentFontName.c_str() };
			m_fontHandles[key] = CreateFontToHandle(fontName, fontSize, -1, DX_FONTTYPE_NORMAL);
			it = m_fontHandles.find(key);
		}
		return GetDrawStringWidthToHandle(text, static_cast<int>(std::strlen(text)), it->second);
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
		DrawExtendGraph(x, y, x + width, y + height, handle, FALSE);
	}
} // namespace infrastructure