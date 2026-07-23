#include "LogUtil.h"
#include <DxLib.h>
#include <cstdio>
#include <ctime>

namespace infrastructure::utility
{
    LogUtil::LogUtil()
        : m_consoleHandle{}
    {
#ifdef _DEBUG
		// Windowsコンソールウィンドウを作成（不要な場合はコメントアウト）
		AllocConsole();

		// 標準出力をコンソールにリダイレクト
		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);

		// コンソールの出力コードページを UTF-8 に設定する。
		// ソースは /utf-8 でコンパイルされ文字列リテラルがUTF-8バイトのため、
		// コンソールもUTF-8にしないと日本語ログが文字化けする
		SetConsoleOutputCP(CP_UTF8);

		// コンソールハンドルを取得
		m_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		// コンソールウィンドウのタイトルを設定
		SetConsoleTitleA("DxLib-3D Debug Console");

		// AllocConsole() は新しいコンソールウィンドウにフォーカスを移してしまい、
		// ゲーム本体ウィンドウが非アクティブになってキー入力を取得できなくなる
		// （Escでポーズが開かない等の不具合の原因になる）。明示的にフォーカスを戻す。
		HWND gameWindow{ GetMainWindowHandle() };
		if (gameWindow)
			SetForegroundWindow(gameWindow);

		// printf("===========================================\n");
		// printf("  DxLib-3D Debug Console\n");
		// printf("===========================================\n\n");
#endif
    }
    
    LogUtil::~LogUtil()
    {
#ifdef _DEBUG
        // コンソールを解放
        FreeConsole();
#endif
    }

    void LogUtil::log(const char* message)
    {
#ifdef _DEBUG
        if (m_consoleHandle)
        {
            SetConsoleTextAttribute(m_consoleHandle, COLOR_WHITE);
            printf("[INFO] %s\n", message);
        }
#endif
    }

    void LogUtil::warning(const char* message)
    {
#ifdef _DEBUG
        if (m_consoleHandle)
        {
            SetConsoleTextAttribute(m_consoleHandle, COLOR_YELLOW);
            printf("[WARN] %s\n", message);
            SetConsoleTextAttribute(m_consoleHandle, COLOR_WHITE);
        }
#endif
    }

    void LogUtil::error(const char* message)
    {
#ifdef _DEBUG
        if (m_consoleHandle)
        {
            SetConsoleTextAttribute(m_consoleHandle, COLOR_RED);
            printf("[ERROR] %s\n", message);
            SetConsoleTextAttribute(m_consoleHandle, COLOR_WHITE);
        }
#endif
    }
} // namespace infrastructure::utility