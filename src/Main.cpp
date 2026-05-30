#include "DxLib.h"
#include "resource.h"
#include "SingletonInitializer.h"
#include "ServiceLocatorInitializer.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "core/interface/IAudioManager.h"
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

	// Effekseer 用の設定
	SetUseDirect3DVersion(DX_DIRECT3D_11); // DirectX 11 を指定
	SetChangeScreenModeGraphicsSystemResetFlag(FALSE); // フルスクリーン切り替え時のリソース保護

	if (DxLib_Init() == -1) return -1;

	// 3D Z-buffer 設定
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	SetMouseDispFlag(TRUE); // リリース・デバッグ問わずマウスカーソルを常時表示
	SetUseLighting(FALSE);

	// Singletonインスタンスを初期化（ServiceLocatorInitializer より先に呼ぶ）
	SingletonInitializer::init();

	// サービスを登録
	ServiceLocatorInitializer::init(screenWidth, screenHeight);

	// ServiceLocatorからSceneManagerを取得
	auto* sceneManager = core::base::ServiceLocator::get<game::scene::SceneManager>();	
	
	// 初期シーンをBiosに設定
	sceneManager->changeScene(game::scene::SceneType::InGame);
	
	while (ProcessMessage() == 0)
	{
		ClearDrawScreen();// 画面クリア

		auto* audio{ core::base::ServiceLocator::get<core::iface::IAudioManager>() };
		if (audio) audio->update();

		sceneManager->update(DELTA_TIME);
		sceneManager->draw();

		ScreenFlip();       // 画面を反映
	}

	// DxLib_End の前にサービスを解放する（Effekseer 等のリソース破棄順序を保証する）
	core::base::ServiceLocator::clear();

	DxLib_End();
	return 0;
}