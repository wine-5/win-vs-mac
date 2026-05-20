#pragma once
#include "core/interface/IPerformanceDataProvider.h"
#include <cstdint>

namespace platform::system
{
	/**
	 * @brief Windows API を使ってタスクマネージャー内のパフォーマンスデータを取得するクラス
	 */
	class WindowsPerformanceProvider : public core::iface::IPerformanceDataProvider
	{
	public:
		WindowsPerformanceProvider();
		~WindowsPerformanceProvider();

		/**
		 * @brief システムデータを更新する
		 */
		void update() override;

		/**
		 * @brief 最新のスナップショットを取得する
		 * @return PerformanceSnapshot
		 */
		[[nodiscard]] core::iface::PerformanceSnapshot getSnapshot() const noexcept override;

	private:
		float m_cpuUsage{};
		float m_memoryUsage{};
		float m_diskActivity{};

		// CPU計算用（前フレームの値）
		int64_t m_prevIdleTime{};   // CPUが何もしていなかった時間
		int64_t m_prevKernelTime{}; // CPUがOSの処理に使った時間 
		int64_t m_prevUserTime{};   // CPUがアプリケーションの処理に使った時間

		// ディスクI/O計算用（前フレームの累積値）
		int64_t m_prevBytesRead{};
		int64_t m_prevBytesWritten{};

		void* m_hDisk{}; // HANDLE(Window.hをヘッダーに漏らさないために void* で保持する）

		void updateCpu();
		void updateMemory();
		void updateDisk();
	};
}