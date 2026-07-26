#pragma once
#include "core/interface/ILogger.h"
#include <fstream>

namespace platform::utility
{
	/**
	 * @brief Windowsコンソールへログを出力するILogger実装
	 *
	 * コンソール制御にWinAPIを使うため、Platform層に置く。
	 * HANDLE を直接持つと Windows.h をヘッダへ露出させ、これをincludeした
	 * 全翻訳単位に min/max マクロ等の汚染を撒くことになる。
	 * そのため WindowsPerformanceProvider と同じく void* で保持し、
	 * WinAPIへの依存は cpp 側に閉じ込める。
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

	  private:
		/**
		 * @brief 1行をコンソールとファイルの両方へ書き出す
		 * @param color コンソールの文字色（実体は WORD）
		 * @param prefix 行頭のラベル
		 * @param message 本文
		 */
		void writeLine(unsigned short color, const char* prefix, const char* message);

		// コンソールのハンドル（実体は HANDLE。Windows.h をヘッダへ出さないため void* で持つ）
		void* m_consoleHandle{ nullptr };

		// ログの控え。フルスクリーンやフリーズ中はコンソールを前面に出せず読めないため、
		// 後から確認できるようファイルにも同じ内容を残す
		std::ofstream m_logFile{};
	};
} // namespace platform::utility
