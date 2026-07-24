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

	/**
	 * @brief 指定色でコンソールへ1行出力する
	 * @param handle コンソールハンドル
	 * @param color 文字色
	 * @param prefix 行頭のラベル
	 * @param message 本文
	 */
	void writeLine(void* handle, WORD color, const char* prefix, const char* message)
	{
		if (handle == nullptr)
			return;

		SetConsoleTextAttribute(static_cast<HANDLE>(handle), color);
		std::printf("%s %s\n", prefix, message);
		SetConsoleTextAttribute(static_cast<HANDLE>(handle), COLOR_WHITE);
	}
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

	void LogUtil::log([[maybe_unused]] const char* message)
	{
#ifdef _DEBUG
		writeLine(m_consoleHandle, COLOR_WHITE, "[INFO]", message);
#endif
	}

	void LogUtil::warning([[maybe_unused]] const char* message)
	{
#ifdef _DEBUG
		writeLine(m_consoleHandle, COLOR_YELLOW, "[WARN]", message);
#endif
	}

	void LogUtil::error([[maybe_unused]] const char* message)
	{
#ifdef _DEBUG
		writeLine(m_consoleHandle, COLOR_RED, "[ERROR]", message);
#endif
	}
} // namespace platform::utility
