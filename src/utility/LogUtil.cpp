#include "LogUtil.h"
#include <DxLib.h>
#include <cstdio>

namespace utility
{
    int LogUtil::m_line = 0;

    void LogUtil::print(int r, int g, int b, const char* format, va_list args)
    {
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), format, args);
        DrawFormatString(0, m_line * m_lineHeight, GetColor(r, g, b), "%s", buffer);
        m_line++;
    }

    void LogUtil::log(const char* format, ...)
    {
#ifdef _DEBUG
        va_list args;
        va_start(args, format);
        print(255, 255, 255, format, args);
        va_end(args);
#endif
    }

    void LogUtil::warning(const char* format, ...)
    {
#ifdef _DEBUG
        va_list args;
        va_start(args, format);
        print(255, 255, 0, format, args);
        va_end(args);
#endif
    }

    void LogUtil::error(const char* format, ...)
    {
#ifdef _DEBUG
        va_list args;
        va_start(args, format);
        print(255, 0, 0, format, args);
        va_end(args);
#endif
    }

    void LogUtil::clear()
    {
#ifdef _DEBUG
        m_line = 0; // 行番号をリセット
#endif
    }
}