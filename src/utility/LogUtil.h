#pragma once
#include <cstdarg>

namespace utility
{
    class LogUtil
    {
    public:
        static void log(const char* format, ...);
        static void warning(const char* format, ...);
        static void error(const char* format, ...);
        static void clear();

    private:
        static void print(int r, int g, int b, const char* format, va_list args);
        static int m_line; // 現在の行番号
        static const int m_lineHeight = 20; // 1行の高さ
    };
}