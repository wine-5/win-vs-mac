// 自前ヘッダを先にincludeする（DxLibのマクロ（DEFAULT_FONT_SIZE等）と定数名の衝突を防ぐ）
#include "Application.h"
#include "core/base/ServiceLocator.h"
#include "DxLib.h"
#include "resource.h"
#include <exception>

namespace
{
	constexpr int COLOR_BIT = 32;
} // namespace

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	int screenWidth{}, screenHeight{};
#ifdef _DEBUG // DEBUG: 開発中はウインドウモードで起動
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

	SetDrawScreen(DX_SCREEN_BACK);  // 描画先を裏画面に設定

	// 3D Z-buffer 設定
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	SetMouseDispFlag(TRUE); // リリース・デバッグ問わずマウスカーソルを常時表示
	SetUseLighting(FALSE);

	try
	{
		// アプリケーション本体（サービス初期化・メインループ・ポーズ制御を統括する）
		// GameManager/PauseManagerの寿命をServiceLocator::clear()より先に終わらせないよう
		// スコープで囲む
		Application app{ screenWidth, screenHeight };
		app.run();

		// DxLib_End の前にサービスを解放する（Effekseer 等のリソース破棄順序を保証する）
		core::base::ServiceLocator::clear();
	}
	catch (const std::exception& e)
	{
		// リソース欠落などの初期化失敗をここで受け止める。
		// 中途半端な状態で起動を続けず、原因を提示して終了する
		core::base::ServiceLocator::clear();
		DxLib_End();
		MessageBoxA(nullptr, e.what(), "起動に失敗しました", MB_OK | MB_ICONERROR);
		return -1;
	}

	DxLib_End();
	return 0;
}