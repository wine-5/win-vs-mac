#include "DxLib.h"
#include "resource.h"
#include "SingletonInitializer.h"
#include "ServiceLocatorInitializer.h"
#include "core/base/ServiceLocator.h"
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

	SetAlwaysRunFlag(TRUE); // ファイルダイアログ等でウィンドウが非アクティブになっても描画を継続する
	SetWindowIconID(IDI_GAMEICON); // アプリケーションアイコンを設定
	SetMainWindowText("Win VS Mac"); // ウィンドウタイトルを設定
	if (DxLib_Init() == -1) return -1;
	SetMouseDispFlag(TRUE); // リリース・デバッグ問わずマウスカーソルを常時表示
	SetUseLighting(FALSE);

	// Singletonインスタンスを初期化（ServiceLocatorInitializer より先に呼ぶ）
	SingletonInitializer::init();

	// サービスを登録
	ServiceLocatorInitializer::init(screenWidth, screenHeight);

	// ServiceLocatorからSceneManagerを取得
	auto* sceneManager = core::base::ServiceLocator::get<game::scene::SceneManager>();
	
	// 初期シーンをBiosに設定
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