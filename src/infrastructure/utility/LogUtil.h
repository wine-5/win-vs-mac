#pragma once
#include "core/interface/ILogger.h"
#include <cstdarg>
#include <Windows.h>

namespace infrastructure::utility
{
    /**
     * @brief 別ウィンドウでログ出力を担当するユーティリティクラス
     */
    class LogUtil : public core::iface::ILogger
    {
    public:
        /**
         * @brief コンストラクタ - Windowsコンソールを作成
         */
        LogUtil();
        
        /**
         * @brief デストラクタ - コンソールを解放
         */
        ~LogUtil() override;

        /**
         * @brief 通常ログを出力する
         * @param message ログメッセージ
         */
        void log(const char* message) override;
        
        /**
         * @brief 警告ログを出力する
         * @param message 警告メッセージ
         */
        void warning(const char* message) override;
        
        /**
         * @brief エラーログを出力する
         * @param message エラーメッセージ
         */
        void error(const char* message) override;
        
        /**
         * @brief ログをクリアする
         */
        void clear() override;

    private:
        HANDLE m_consoleHandle;  // コンソールのハンドル
        
        // コンソールの色定数
        static constexpr WORD COLOR_WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        static constexpr WORD COLOR_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        static constexpr WORD COLOR_RED = FOREGROUND_RED | FOREGROUND_INTENSITY;
    };
}