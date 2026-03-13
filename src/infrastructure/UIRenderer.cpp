#include "UIRenderer.h"
#include "DxLib.h"
#include <cstring>

namespace infrastructure
{
	void UIRenderer::drawBox(int x, int y, int width, int height, unsigned int color, bool isFilled)
	{
		if (isFilled)
			DrawBox(x, y, x + width, y + height, color, TRUE);
		else
			DrawBox(x, y, x + width, y + height, color, FALSE);
	}

	void UIRenderer::drawText(int x, int y, const char* text, unsigned int color)
	{
		DrawString(x, y, text, color);
	}

	int UIRenderer::getTextWidth(const char* text) const
	{
		return GetDrawStringWidth(text, static_cast<int>(std::strlen(text)));
	}
}