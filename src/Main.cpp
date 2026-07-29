// 自前ヘッダを先にincludeする（DxLibのマクロ（DEFAULT_FONT_SIZE等）と定数名の衝突を防ぐ）
#include "Application.h"
#include "core/base/ServiceLocator.h"
#include "core/constant/DebugFlags.h"
#include "DxLib.h"
#include "resource.h"
#include "core/interface/IMemoryProbe.h"
#include "core/utility/Probe.h"
#include "platform/diagnostics/MemoryProbe.h" // 一時: メモリ調査用（原因特定後に削除）
#include <exception>
#include <memory>

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
	// 描画の高さは 1080 に固定し（UIスケールと負荷を端末に依らず一定に保つ）、
	// 幅だけデスクトップのアスペクト比に合わせる。これで枠なしウィンドウをデスクトップ全体へ
	// 拡大したとき、余白（レターボックス）も歪み（ストレッチ）も無くどの端末でも全画面になる。
	// ＝ Unity の Fullscreen Window と同じ考え方
	// 計測プローブは他のどのサービスより先に登録する。以降はGame層・Infrastructure層からも
	// core::probe 越しに呼べるようになり、Platform層へ直接依存せずに計測できる
	core::base::ServiceLocator::provide<core::iface::IMemoryProbe>(
	    std::make_unique<platform::diagnostics::MemoryProbe>());
	core::base::ServiceLocator::get<core::iface::IMemoryProbe>()->reset();
	core::probe::mark("WinMain 開始");

	int screenWidth{ RENDER_WIDTH };
	int screenHeight{ RENDER_HEIGHT };

#ifndef _DEBUG
	double fullscreenRate{ 1.0 }; // 描画バッファをデスクトップ全体へ広げる拡大率
	{
		DEVMODE displayMode{};
		displayMode.dmSize = sizeof(displayMode);
		if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &displayMode) != 0 && displayMode.dmPelsHeight > 0)
		{
			// 高さは固定、幅をデスクトップのアスペクト比に合わせる（縦横比が一致するので歪まない）
			screenWidth = static_cast<int>(RENDER_HEIGHT * static_cast<double>(displayMode.dmPelsWidth) / displayMode.dmPelsHeight);
			screenHeight = RENDER_HEIGHT;
			fullscreenRate = static_cast<double>(displayMode.dmPelsHeight) / RENDER_HEIGHT;
		}
	}
#endif

	SetGraphMode(screenWidth, screenHeight, COLOR_BIT);

#ifdef _DEBUG // DEBUG: 開発中はウインドウモードで起動する（画面に収まるよう縮小表示する）
	ChangeWindowMode(TRUE);
	SetWindowSizeExtendRate(calcDebugWindowExtendRate());
#else // リリース：ボーダーレス全画面（枠なしウィンドウでデスクトップ全体を覆う）
	// 排他フルスクリーンにすると、セレクト/リザルトのWebViewなど別ウィンドウが前面に出るたびに
	// 前面を奪い合い、画面が切り替わって入力が他アプリへ行ってしまう。枠なしウィンドウにすれば
	// 見た目は全画面のまま、他ウィンドウと素直に重なるためこの問題が起きない
	ChangeWindowMode(TRUE);
	SetWindowStyleMode(4); // タイトルバー・枠の無いスタイル
	SetWindowSizeExtendRate(fullscreenRate);
	SetWindowPosition(0, 0); // デスクトップ左上に合わせて全体を覆う
#endif

	SetAlwaysRunFlag(TRUE); // ファイルダイアログ等でウィンドウが非アクティブになっても描画を継続する
	SetWindowIconID(IDI_GAMEICON); // アプリケーションアイコンを設定
	SetMainWindowText("Win VS Mac"); // ウィンドウタイトルを設定

	// DxLibの Log.txt を書き出すか。DxLib_Init より前でしか変えられない
	SetOutApplicationLogValidFlag(core::constant::WRITE_DEBUG_LOG_FILES ? TRUE : FALSE);

	// Effekseer 用の設定
	SetUseDirect3DVersion(DX_DIRECT3D_11); // DirectX 11 を指定
	SetChangeScreenModeGraphicsSystemResetFlag(FALSE); // フルスクリーン切り替え時のリソース保護

	if (DxLib_Init() == -1) return -1;

	core::probe::mark("DxLib_Init 完了");

	// 枠なしスタイル（SetWindowStyleMode(4)）だと WS_POPUP になり、シェルがタスクバー項目を
	// 出してくれないことがある。セレクト画面のサブウィンドウが代わりに Alt+Tab の代表として
	// 拾われ、他アプリへ切り替えるとゲームへ戻る手段が無くなるため、本体を明示的に
	// 「タスクバーに出るアプリのウィンドウ」として宣言しておく
	if (HWND mainHwnd{ GetMainWindowHandle() })
	{
		const LONG_PTR exStyle{ GetWindowLongPtrW(mainHwnd, GWL_EXSTYLE) };
		SetWindowLongPtrW(mainHwnd, GWL_EXSTYLE, exStyle | WS_EX_APPWINDOW);
		// タスクバー項目の有無はウィンドウが表示される瞬間に決まる。
		// 表示済みのまま拡張スタイルを変えても反映されないので、隠して出し直す
		ShowWindow(mainHwnd, SW_HIDE);
		ShowWindow(mainHwnd, SW_SHOW);
	}

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