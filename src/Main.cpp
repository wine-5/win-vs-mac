#include "DxLib.h"
#include "engine/scene/InGameScene.h"
#include "core/utility/LogUtil.h"
#include "core/ServiceLocator.h"
#include "engine/Camera.h"  
#include "engine/Renderer.h"

namespace
{
	constexpr float TARGET_FPS = 60.0f;
	constexpr float DELTA_TIME = 1.0f / TARGET_FPS;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	if (DxLib_Init() == -1) return -1;

	// ServiceLocatorにサービスを登録
	core::ServiceLocator::provide(std::make_unique<engine::Camera>());
	core::ServiceLocator::provide(std::make_unique<engine::Renderer>());

	engine::scene::InGameScene inGameScene;

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