#pragma once
#include <cstdarg>

namespace core::utility
{
    class LogUtil
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

        static void log(const char* format, ...);
        static void warning(const char* format, ...);
        static void error(const char* format, ...);
        static void clear();

    private:
        static void print(int r, int g, int b, const char* format, va_list args);
        static int s_line; // 現在の行番号
        static constexpr int LINE_HEIGHT = 20; // 1行の高さ
    };
}