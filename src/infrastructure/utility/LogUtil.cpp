#include "LogUtil.h"
#include <cstdio>
#include <ctime>

namespace infrastructure::utility
{
    LogUtil::LogUtil()
        : m_consoleHandle{}
    {
#ifdef _DEBUG
		//  //Windowsコンソールウィンドウを作成（不要な場合はコメントアウト）
		// AllocConsole();

		// // 標準出力をコンソールにリダイレクト
		// FILE* fp;
		// freopen_s(&fp, "CONOUT$", "w", stdout);
		// freopen_s(&fp, "CONOUT$", "w", stderr);

		// // コンソールの出力コードページを Shift-JIS に設定（DxLibのボーン名等がShift-JISのため）
		// SetConsoleOutputCP(932);

		// // コンソールハンドルを取得
		// m_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		// // コンソールウィンドウのタイトルを設定
		// SetConsoleTitleA("DxLib-3D Debug Console");

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

    void LogUtil::clear()
    {
#ifdef _DEBUG
        // コンソールは自動クリアしない（履歴を残す）
        // 必要ならsystem("cls")を使用
#endif
    }
} // namespace infrastructure::utility