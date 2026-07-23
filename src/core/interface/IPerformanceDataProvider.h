#pragma once

namespace core::iface
{
	/**
	 * @brief タスクマネージャー内のパフォーマンスのスナップショット
	 */
	struct PerformanceSnapshot
	{
		float cpuUsage{};     // システム全体のCPU使用率(0.0f～1.0f)
		float memoryUsage{};  // システム全体のメモリ使用率(0.0f～1.0f)
		float diskActivity{}; // システム全体のディスクI/O活性度(0.0f～1.0f)

		// DEBUG: 負荷の原因特定用（このゲーム自身のプロセス単体の値。リリース時に削除）
		float processCpuUsage{};      // このプロセスのCPU使用率(0.0f～1.0f。システム全体に対する占有率)
		float processMemoryUsageMB{}; // このプロセスのメモリ使用量（ワーキングセットサイズ、MB）
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