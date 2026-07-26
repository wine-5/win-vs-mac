#include "LogUtil.h"
#include <Windows.h>
#include <cstdio>

namespace
{
#ifdef _DEBUG
	// コンソールの色定数
	constexpr WORD COLOR_WHITE{ FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY };
	constexpr WORD COLOR_YELLOW{ FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY };
	constexpr WORD COLOR_RED{ FOREGROUND_RED | FOREGROUND_INTENSITY };

	// ログの控えを書き出すファイル。DxLibが出力する Log.txt とは別にする
	constexpr const char* LOG_FILE_PATH{ "game_log.txt" };
#endif
} // namespace

namespace platform::utility
{
	LogUtil::LogUtil()
	{
#ifdef _DEBUG
		// AllocConsole() は新しいコンソールへフォーカスを移してしまい、ゲーム本体ウィンドウが
		// 非アクティブになってキー入力を取得できなくなる（Escでポーズが開かない等）。
		// 生成前の前面ウィンドウ（＝ゲーム本体）を控えておき、あとで戻す。
		HWND previousForeground{ GetForegroundWindow() };

		// Windowsコンソールウィンドウを作成（不要な場合はコメントアウト）
		AllocConsole();

		// 標準出力をコンソールにリダイレクト
		FILE* fp{};
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);

		// コンソールの出力コードページを UTF-8 に設定する。
		// ソースは /utf-8 でコンパイルされ文字列リテラルがUTF-8バイトのため、
		// コンソールもUTF-8にしないと日本語ログが文字化けする
		SetConsoleOutputCP(CP_UTF8);

		m_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		SetConsoleTitleA("DxLib-3D Debug Console");

		// 起動ごとに作り直す（前回の内容が混ざるとどの実行のログか分からなくなる）
		m_logFile.open(LOG_FILE_PATH, std::ios::out | std::ios::trunc);

		if (previousForeground != nullptr)
			SetForegroundWindow(previousForeground);
#endif
	}

	LogUtil::~LogUtil()
	{
#ifdef _DEBUG
		// コンソールを解放
		FreeConsole();
#endif
	}

	void LogUtil::writeLine([[maybe_unused]] unsigned short color,
	    [[maybe_unused]] const char* prefix, [[maybe_unused]] const char* message)
	{
#ifdef _DEBUG
		if (m_consoleHandle != nullptr)
		{
			SetConsoleTextAttribute(static_cast<HANDLE>(m_consoleHandle), color);
			std::printf("%s %s\n", prefix, message);
			SetConsoleTextAttribute(static_cast<HANDLE>(m_consoleHandle), COLOR_WHITE);
		}

		// 1行ごとに書き出す。落ちた直前の行まで残さないと原因を追えない
		if (m_logFile.is_open())
			m_logFile << prefix << ' ' << message << '\n'
			          << std::flush;
#endif
	}

	void LogUtil::log([[maybe_unused]] const char* message)
	{
#ifdef _DEBUG
		writeLine(COLOR_WHITE, "[INFO]", message);
#endif
	}

	void LogUtil::warning([[maybe_unused]] const char* message)
	{
#ifdef _DEBUG
		writeLine(COLOR_YELLOW, "[WARN]", message);
#endif
	}

	void LogUtil::error([[maybe_unused]] const char* message)
	{
#ifdef _DEBUG
		writeLine(COLOR_RED, "[ERROR]", message);
#endif
	}
} // namespace platform::utility
