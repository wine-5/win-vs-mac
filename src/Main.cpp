#include "DxLib.h"
#include "ServiceLocatorInitializer.h"
#include "core/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "game/scene/SceneManager.h"
#include "game/scene/SceneType.h"

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

	ServiceLocatorInitializer::init(SCREEN_WIDTH, SCREEN_HEIGHT);

	// ServiceLocatorからSceneManagerを取得
	auto* sceneManager = core::ServiceLocator::get<game::scene::SceneManager>();
	
	// 初期シーンをTitleに設定
	sceneManager->changeScene(game::scene::SceneType::Title);

	while (ProcessMessage() == 0)
	{
		ClearDrawScreen();// 画面クリア
		
		sceneManager->update(DELTA_TIME);
		sceneManager->draw();

		ScreenFlip();       // 画面を反映
	}

	DxLib_End();
	return 0;
}