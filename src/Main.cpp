#include "DxLib.h"
#include "game/scene/InGameScene.h"
#include "utility/LogUtil.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	if (DxLib_Init() == -1) return -1;

	game::scene::InGameScene inGameScene;

	while (ProcessMessage() == 0)
	{
		ClearDrawScreen();// 画面クリア
		utility::LogUtil::clear();
		float deltaTime = 1.0f / 60.0f;
		inGameScene.update(deltaTime);

		ScreenFlip();       // 画面を反映
	}

	DxLib_End();
	return 0;
}