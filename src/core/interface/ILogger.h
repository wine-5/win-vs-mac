#pragma once
#include <format>
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
} // namespace core::iface

// ログ出力用マクロ（std::formatベースで型安全。書式は "{}" プレースホルダを使う）
// 書式と引数の不一致はコンパイルエラーになる（consteval な format 文字列チェック）
#define LOG(...) \
	core::base::ServiceLocator::get<core::iface::ILogger>()->log(std::format(__VA_ARGS__).c_str())

#define LOG_W(...) \
	core::base::ServiceLocator::get<core::iface::ILogger>()->warning(std::format(__VA_ARGS__).c_str())

#define LOG_E(...) \
	core::base::ServiceLocator::get<core::iface::ILogger>()->error(std::format(__VA_ARGS__).c_str())
