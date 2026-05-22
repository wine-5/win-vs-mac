#pragma once
#include <cstdarg>
#include "core/base/ServiceLocator.h"

namespace core::iface
{
    /**
     * @brief ログ出力の純粋仮想クラス
     * Game層がInfrastructure層（DxLib）に直接依存しないための抽象化
     */
    class ILogger
    {
    public:
        virtual ~ILogger() = default;
        
        /**
         * @brief 通常ログを出力する
         * @param message ログメッセージ
         */
        virtual void log(const char* message) = 0;
        
        /**
         * @brief 警告ログを出力する
         * @param message 警告メッセージ
         */
        virtual void warning(const char* message) = 0;
        
        /**
         * @brief エラーログを出力する
         * @param message エラーメッセージ
         */
        virtual void error(const char* message) = 0;
        
        /**
         * @brief ログをクリアする
         */
        virtual void clear() = 0;
    };
}

// ログ出力用マクロ（使用を簡潔にする）
// ログ出力用マクロ（使用を簡潔にする）
#define LOG(...) \
    do { \
        char _buf[1024]; \
        snprintf(_buf, sizeof(_buf), __VA_ARGS__); \
        core::base::ServiceLocator::get<core::iface::ILogger>()->log(_buf); \
    } while(0)

#define LOG_W(...) \
    do { \
        char _buf[1024]; \
        snprintf(_buf, sizeof(_buf), __VA_ARGS__); \
        core::base::ServiceLocator::get<core::iface::ILogger>()->warning(_buf); \
    } while(0)

#define LOG_E(...) \
    do { \
        char _buf[1024]; \
        snprintf(_buf, sizeof(_buf), __VA_ARGS__); \
        core::base::ServiceLocator::get<core::iface::ILogger>()->error(_buf); \
    } while(0)
