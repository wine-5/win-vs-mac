#pragma once

namespace core::iface
{
	/**
	 * @brief ログ出力の純粋仮想クラス
	 * Game層がInfrastructure層（DxLib）に直接依存しないための抽象化
	 *
	 * 書式付きで出力したい場合は core/utility/Log.h の core::log::info / warn / error を使う。
	 * このヘッダは実装への依存を持たない純粋な宣言に保つこと。
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
    };
} // namespace core::iface
