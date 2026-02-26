#include "DxLib.h"
#include "game/scene/InGameScene.h"
#include "utility/LogUtil.h"

namespace
{
	constexpr float TARGET_FPS = 60.0f;
	constexpr float DELTA_TIME = 1.0f / TARGET_FPS;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	if (DxLib_Init() == -1) return -1;

	game::scene::InGameScene inGameScene;

	while (ProcessMessage() == 0)
	{
		ClearDrawScreen();// 画面クリア
		utility::LogUtil::clear();
		inGameScene.update(DELTA_TIME);

		ScreenFlip();       // 画面を反映
	}

	DxLib_End();
	return 0;
}