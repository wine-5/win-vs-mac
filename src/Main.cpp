// 自前ヘッダを先にincludeする（DxLibのマクロ（DEFAULT_FONT_SIZE等）と定数名の衝突を防ぐ）
#include "Application.h"
#include "core/base/ServiceLocator.h"
#include "DxLib.h"
#include "resource.h"
#include <exception>

namespace
{
	constexpr int COLOR_BIT = 32;

	// 描画解像度は常にこの値で固定する。
	// フルスクリーンでもモニタのネイティブ解像度（4K等）では描かず、この解像度で描いた画面を
	// デスクトップ解像度へ拡大表示する。ピクセル処理量が青天井にならず、どのモニタでも
	// 負荷とUIレイアウトが一定になる
	constexpr int RENDER_WIDTH = 1920;
	constexpr int RENDER_HEIGHT = 1080;

#ifdef _DEBUG
	// DEBUG: 開発中のウィンドウが画面に対して占める割合（タスクバーとタイトルバーのぶん余らせる）
	constexpr double DEBUG_WINDOW_SCREEN_RATIO = 0.8;
	// DEBUG: 表示倍率の下限・上限（極端な解像度でも常識的なサイズに収める）
	constexpr double DEBUG_WINDOW_MIN_RATE = 0.5;
	constexpr double DEBUG_WINDOW_MAX_RATE = 2.0;

	/**
	 * @brief DEBUG: 開発用ウィンドウの表示倍率を、実際のモニタ解像度から求める
	 *
	 * SetWindowSizeExtendRate が扱うのは物理ピクセル数のため、DPIスケーリングの影響を
	 * 受けない EnumDisplaySettings で実解像度を取得する（GetSystemMetrics はプロセスの
	 * DPI認識状態によって論理座標を返すことがあり、高DPI機でウィンドウが極端に小さくなる）。
	 * @return 描画解像度に掛ける表示倍率
	 */
	double calcDebugWindowExtendRate()
	{
		DEVMODE displayMode{};
		displayMode.dmSize = sizeof(displayMode);
		if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &displayMode) == 0)
			return 1.0;

		const double rateX{ DEBUG_WINDOW_SCREEN_RATIO * displayMode.dmPelsWidth / RENDER_WIDTH };
		const double rateY{ DEBUG_WINDOW_SCREEN_RATIO * displayMode.dmPelsHeight / RENDER_HEIGHT };
		const double rate{ rateX < rateY ? rateX : rateY };

		if (rate < DEBUG_WINDOW_MIN_RATE)
			return DEBUG_WINDOW_MIN_RATE;
		if (rate > DEBUG_WINDOW_MAX_RATE)
			return DEBUG_WINDOW_MAX_RATE;
		return rate;
	}
#endif
} // namespace

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// 描画解像度はDebug/Releaseで揃える（違うとUIの座標ズレや負荷差がビルド構成に依存してしまう）。
	// フルスクリーン時はこの解像度で描いた画面をデスクトップ解像度へバイリニア拡大する。
	// モニタの画面モードを切り替えないため、切り替え時のちらつきも起きない
	const int screenWidth{ RENDER_WIDTH };
	const int screenHeight{ RENDER_HEIGHT };
	SetGraphMode(screenWidth, screenHeight, COLOR_BIT);
	SetFullScreenResolutionMode(DX_FSRESOLUTIONMODE_DESKTOP);
	SetFullScreenScalingMode(DX_FSSCALINGMODE_BILINEAR);

#ifdef _DEBUG // DEBUG: 開発中はウインドウモードで起動する（画面に収まるよう縮小表示する）
	ChangeWindowMode(TRUE);
	SetWindowSizeExtendRate(calcDebugWindowExtendRate());
#else // リリース用（フルスクリーン）
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