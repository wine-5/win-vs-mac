#include "LogUtil.h"
#include <Windows.h>
#include <crtdbg.h>
#include <cstdio>

namespace
{
	// コンソールの色定数
	constexpr WORD COLOR_WHITE{ FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY };
	constexpr WORD COLOR_YELLOW{ FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY };
	constexpr WORD COLOR_RED{ FOREGROUND_RED | FOREGROUND_INTENSITY };

	// ログの控えを書き出すファイル。DxLibが出力する Log.txt とは別にする
	constexpr const char* LOG_FILE_PATH{ "game_log.txt" };

#ifdef _DEBUG
	// CRTのデバッグアサーション（範囲外アクセス等）の書き出し先
	constexpr const char* ASSERT_FILE_PATH{ "assert_log.txt" };
#endif
} // namespace

namespace platform::utility
{
	LogUtil::LogUtil()
	{
		// 控えのファイルはリリースでも必ず開く。全画面表示のままフリーズすると
		// コンソールもダイアログも前面に出せず、後から読めるのはこれだけになる。
		// 起動ごとに作り直す（前回の内容が混ざるとどの実行のログか分からなくなる）
		m_logFile.open(LOG_FILE_PATH, std::ios::out | std::ios::trunc);

#ifdef _DEBUG
		// AllocConsole() は新しいコンソールへフォーカスを移してしまい、ゲーム本体ウィンドウが
		// 非アクティブになってキー入力を取得できなくなる（Escでポーズが開かない等）。
		// 生成前の前面ウィンドウ（＝ゲーム本体）を控えておき、あとで戻す。
		// HWND previousForeground{ GetForegroundWindow() };

		//// Windowsコンソールウィンドウを作成（不要な場合はコメントアウト）
		// AllocConsole();

		//// 標準出力をコンソールにリダイレクト
		// FILE* fp{};
		// freopen_s(&fp, "CONOUT$", "w", stdout);
		// freopen_s(&fp, "CONOUT$", "w", stderr);

		//// コンソールの出力コードページを UTF-8 に設定する。
		//// ソースは /utf-8 でコンパイルされ文字列リテラルがUTF-8バイトのため、
		//// コンソールもUTF-8にしないと日本語ログが文字化けする
		// SetConsoleOutputCP(CP_UTF8);

		// m_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		// SetConsoleTitleA("DxLib-3D Debug Console");

		//// CRTのデバッグアサーション（std::arrayの範囲外など）はロガーを通らず
		//// ダイアログを出すだけで終わる。ゲームがフルスクリーンだとそのダイアログが
		//// 前面に出せず内容を読めないため、内容をファイルへも書き出させる
		// m_assertFileHandle = CreateFileA(ASSERT_FILE_PATH, GENERIC_WRITE,
		//     FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS,
		//     FILE_ATTRIBUTE_NORMAL, nullptr);
		// if (m_assertFileHandle != INVALID_HANDLE_VALUE)
		//{
		//	for (const int reportType : { _CRT_ASSERT, _CRT_ERROR, _CRT_WARN })
		//	{
		//		// ダイアログも残す（デバッガで止めたいときのため）。ファイルへは常に出す
		//		_CrtSetReportMode(reportType, _CRTDBG_MODE_FILE | _CRTDBG_MODE_WNDW | _CRTDBG_MODE_DEBUG);
		//		_CrtSetReportFile(reportType, static_cast<HANDLE>(m_assertFileHandle));
		//	}
		// }

		// if (previousForeground != nullptr)
		//	SetForegroundWindow(previousForeground);
#endif
	}

	LogUtil::~LogUtil()
	{
#ifdef _DEBUG
		if (m_assertFileHandle != nullptr && m_assertFileHandle != INVALID_HANDLE_VALUE)
			CloseHandle(static_cast<HANDLE>(m_assertFileHandle));

		// コンソールを解放
		FreeConsole();
#endif
	}

	void LogUtil::writeLine([[maybe_unused]] unsigned short color,
	    const char* prefix, const char* message)
	{
		const std::lock_guard<std::mutex> lock{ m_writeMutex };

#ifdef _DEBUG
		if (m_consoleHandle != nullptr)
		{
			SetConsoleTextAttribute(static_cast<HANDLE>(m_consoleHandle), color);
			std::printf("%s %s\n", prefix, message);
			SetConsoleTextAttribute(static_cast<HANDLE>(m_consoleHandle), COLOR_WHITE);
		}
#endif

		// デバッガの出力ウィンドウ（や DebugView）へも流す。
		// 全画面のままフリーズしても、こちらは別プロセスから読めるため
		// ウィンドウを切り替えずに状況を追える
		OutputDebugStringA(prefix);
		OutputDebugStringA(" ");
		OutputDebugStringA(message);
		OutputDebugStringA("\n");

		// 1行ごとに書き出す。落ちた直前の行まで残さないと原因を追えない
		if (m_logFile.is_open())
			m_logFile << prefix << ' ' << message << '\n'
			          << std::flush;
	}

	void LogUtil::log([[maybe_unused]] const char* message)
	{
		// 通常ログは量が多く、毎行flushすると実行速度に響くためデバッグ時のみ
#ifdef _DEBUG
		writeLine(COLOR_WHITE, "[INFO]", message);
#endif
	}

	void LogUtil::warning(const char* message)
	{
		writeLine(COLOR_YELLOW, "[WARN]", message);
	}

	void LogUtil::error(const char* message)
	{
		writeLine(COLOR_RED, "[ERROR]", message);
	}
} // namespace platform::utility
