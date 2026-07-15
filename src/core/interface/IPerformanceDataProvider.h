#pragma once

namespace core::iface
{
	/**
	 * @brief タスクマネージャー内のパフォーマンスのスナップショット
	 */
	struct PerformanceSnapshot
	{
		float cpuUsage{};     // CPU使用率(0.0f～1.0f)     
		float memoryUsage{};  // メモリ使用率(0.0f～1.0f)
		float diskActivity{}; // ディスクI/O活性度(0.0f～1.0f) 
	};

	/**
	 * @brief パフォーマンスデータ取得の抽象インターフェース
	 */
	class IPerformanceDataProvider
	{
	public:
		virtual ~IPerformanceDataProvider() = default;


		/**
		 * @brief システムデータを更新する
		 */
		virtual void update() = 0;

		/**
		 * @brief 最新のスナップショットを取得する
		 * @return PerformanceSnapshot
		 */
		[[nodiscard]] virtual PerformanceSnapshot getSnapshot() const noexcept = 0;
	};
} // namespace core::iface