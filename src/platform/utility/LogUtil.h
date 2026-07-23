#pragma once
#include "core/interface/ILogger.h"

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
		// コンソールのハンドル（実体は HANDLE。Windows.h をヘッダへ出さないため void* で持つ）
		void* m_consoleHandle{ nullptr };
	};
} // namespace platform::utility
