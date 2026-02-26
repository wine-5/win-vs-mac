#pragma once

namespace utility
{
    /**
     * @brief ログを出力するユーティリティクラス
     */
    class LogUtil
    {
    public:
        static void debug(const char* format, ...);
        static void clear();
    };
}