#pragma once
#include "core/interface/ILogger.h"
#include <cstdarg>

namespace infrastructure::utility
{
    /**
     * @brief 画面へのログ出力を担当するユーティリティクラス
     */
    class LogUtil : public core::iface::ILogger
    {
    public:
        static constexpr int BUFFER_SIZE = 1024;
        
        // ログの色定数
        static constexpr int LOG_COLOR_R = 255;
        static constexpr int LOG_COLOR_G = 255;
        static constexpr int LOG_COLOR_B = 255;
        
        static constexpr int WARNING_COLOR_R = 255;
        static constexpr int WARNING_COLOR_G = 255;
        static constexpr int WARNING_COLOR_B = 0;
        
        static constexpr int ERROR_COLOR_R = 255;
        static constexpr int ERROR_COLOR_G = 0;
        static constexpr int ERROR_COLOR_B = 0;

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
        static void print(int r, int g, int b, const char* message);
        static int s_line; // 現在の行番号
        static constexpr int LINE_HEIGHT = 20; // 1行の高さ
    };
}