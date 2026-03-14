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
	constexpr int   COLOR_BIT = 32;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	int screenWidth{}, screenHeight{};
#ifdef _DEBUG // デバック用
	screenWidth = 1280;
	screenHeight = 720;
	SetGraphMode(screenWidth, screenHeight, COLOR_BIT);
	ChangeWindowMode(TRUE);
#else // リリース用（フルサイズ）
	GetDefaultState(&screenWidth, &screenHeight, nullptr);
	SetGraphMode(screenWidth, screenHeight, COLOR_BIT);
	ChangeWindowMode(FALSE);
#endif

	if (DxLib_Init() == -1) return -1;
	SetUseLighting(FALSE);

	ServiceLocatorInitializer::init(screenWidth, screenHeight);

	// ServiceLocatorからSceneManagerを取得
	auto* sceneManager = core::ServiceLocator::get<game::scene::SceneManager>();
	
	// 初期シーンをTitleに設定
	//sceneManager->changeScene(game::scene::SceneType::Title);

	// デバック用：初期シーンをgameに設定
	sceneManager->changeScene(game::scene::SceneType::InGame);
	
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