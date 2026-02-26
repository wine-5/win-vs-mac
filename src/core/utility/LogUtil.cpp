#include "LogUtil.h"
#include <DxLib.h>
#include <cstdio>

namespace core::utility
{
    int LogUtil::s_line = 0;

    void LogUtil::print(int r, int g, int b, const char* format, va_list args)
    {
        char buffer[BUFFER_SIZE];
        vsnprintf(buffer, sizeof(buffer), format, args);
        DrawFormatString(0, s_line * LINE_HEIGHT, GetColor(r, g, b), "%s", buffer);
        s_line++;
    }

    void LogUtil::log(const char* format, ...)
    {
#ifdef _DEBUG
        va_list args;
        va_start(args, format);
        print(LOG_COLOR_R, LOG_COLOR_G, LOG_COLOR_B, format, args);
        va_end(args);
#endif
    }

    void LogUtil::warning(const char* format, ...)
    {
#ifdef _DEBUG
        va_list args;
        va_start(args, format);
        print(WARNING_COLOR_R, WARNING_COLOR_G, WARNING_COLOR_B, format, args);
        va_end(args);
#endif
    }

    void LogUtil::error(const char* format, ...)
    {
#ifdef _DEBUG
        va_list args;
        va_start(args, format);
        print(ERROR_COLOR_R, ERROR_COLOR_G, ERROR_COLOR_B, format, args);
        va_end(args);
#endif
    }

    void LogUtil::clear()
    {
#ifdef _DEBUG
        s_line = 0; // 行番号をリセット
#endif
    }
}