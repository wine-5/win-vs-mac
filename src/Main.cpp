#include "DxLib.h"
#include "game/scene/InGameScene.h"
#include "core/utility/LogUtil.h"
#include "core/ServiceLocator.h"
#include "infrastructure/Camera.h"  
#include "infrastructure/Renderer.h"
#include "infrastructure/ResourceManager.h"

namespace
{
	constexpr float TARGET_FPS = 60.0f;
	constexpr float DELTA_TIME = 1.0f / TARGET_FPS;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	SetGraphMode(1280, 720, 32);
	ChangeWindowMode(TRUE);

	if (DxLib_Init() == -1) return -1;
	SetUseLighting(FALSE);

	// ServiceLocatorにサービスを登録
	core::ServiceLocator::provide(std::make_unique<infrastructure::Camera>());
	core::ServiceLocator::provide(std::make_unique<infrastructure::Renderer>());
	core::ServiceLocator::provide<core::IResourceManager>(
		std::make_unique<infrastructure::ResourceManager>()
	);
	
	game::scene::InGameScene inGameScene;

	while (ProcessMessage() == 0)
	{
		ClearDrawScreen();// 画面クリア
		core::utility::LogUtil::clear();
		inGameScene.update(DELTA_TIME);

		ScreenFlip();       // 画面を反映
	}

	DxLib_End();
	return 0;
}