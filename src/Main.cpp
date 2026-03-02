#include "DxLib.h"
#include "game/scene/InGameScene.h"
#include "infrastructure/utility/LogUtil.h"

namespace
{
	constexpr float TARGET_FPS = 60.0f;
	constexpr float DELTA_TIME = 1.0f / TARGET_FPS;
	constexpr int   SCREEN_WIDTH = 1280;
	constexpr int   SCREEN_HEIGHT = 720;
	constexpr int   COLOR_BIT = 32;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	SetGraphMode(SCREEN_WIDTH, SCREEN_HEIGHT,COLOR_BIT);
	ChangeWindowMode(TRUE);

	if (DxLib_Init() == -1) return -1;
	SetUseLighting(FALSE);

	// 現在は空実装のためコメントアウト
	//ServiceLocatorInitializer::init();
	game::scene::InGameScene inGameScene;

	while (ProcessMessage() == 0)
	{
		ClearDrawScreen();// 画面クリア
		infrastructure::utility::LogUtil::clear();
		inGameScene.update(DELTA_TIME);

		ScreenFlip();       // 画面を反映
	}

	DxLib_End();
	return 0;
}