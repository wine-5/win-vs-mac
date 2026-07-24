#include "TextureCanvas.h"
#include <DxLib.h>

namespace infrastructure::graphics
{
	int TextureCanvas::create(int width, int height)
	{
		// アルファ無しで作る（壁に貼る不透明なテクスチャのため）
		constexpr int NO_ALPHA{ FALSE };
		return MakeScreen(width, height, NO_ALPHA);
	}

	void TextureCanvas::beginDraw(int canvasHandle, int r, int g, int b)
	{
		if (canvasHandle == -1)
			return;

		SetDrawScreen(canvasHandle);

		// ClearDrawScreen は SetBackgroundColor の色で塗るため、消去色を一時的に差し替える。
		// 元へ戻さないと本編の背景（黒い虚無）まで変わってしまう
		int prevR{ 0 }, prevG{ 0 }, prevB{ 0 };
		GetBackgroundColor(&prevR, &prevG, &prevB);
		SetBackgroundColor(r, g, b);
		ClearDrawScreen();
		SetBackgroundColor(prevR, prevG, prevB);
	}

	void TextureCanvas::endDraw()
	{
		SetDrawScreen(DX_SCREEN_BACK);
	}

	void TextureCanvas::applyToModel(int modelHandle, int canvasHandle)
	{
		if (modelHandle == -1 || canvasHandle == -1)
			return;

		// 配置物は単一テクスチャの立方体を想定している
		const int textureNum{ MV1GetTextureNum(modelHandle) };
		for (int i{ 0 }; i < textureNum; ++i)
			MV1SetTextureGraphHandle(modelHandle, i, canvasHandle, FALSE);
	}

	void TextureCanvas::destroy(int canvasHandle)
	{
		if (canvasHandle == -1)
			return;

		DeleteGraph(canvasHandle);
	}
} // namespace infrastructure::graphics
