#include "WindowsPerformanceProvider.h"
#include <Windows.h>
#include <winioctl.h>

namespace platform::system
{


	WindowsPerformanceProvider::WindowsPerformanceProvider()
		{
			// 物理ディスク0（最初のHDD/SSD）へのハンドルを取得
			HANDLE h{ CreateFileW(
				L"\\\\.\\PhysicalDrive0",
				0,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				nullptr,
				OPEN_EXISTING,
				0,
				nullptr) };

			if (h != INVALID_HANDLE_VALUE) // CreateFileWが失敗したときにINVALID_HANDLE_VALUEを返す
				m_hDisk = h;

			// 前フレーム値を初期化するために初回更新
			update();
		}

		WindowsPerformanceProvider::~WindowsPerformanceProvider()
	{
		// ハンドルが有効な場合、ディスクハンドルを解放する
		if (m_hDisk)
			CloseHandle(static_cast<HANDLE>(m_hDisk));
	}

	void WindowsPerformanceProvider::update()
	{
		updateCpu();
		updateMemory();
		updateDisk();
	}

	core::iface::PerformanceSnapshot WindowsPerformanceProvider::getSnapshot() const noexcept
	{
		return { m_cpuUsage,m_memoryUsage,m_diskActivity };
	}

	void WindowsPerformanceProvider::updateCpu()
	{
		FILETIME idleTime{}, kernelTime{}, userTime{};
		if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) return;

		// FILETIME構造体をint64_tに変換
		auto toInt64 = [](const FILETIME& ft) -> int64_t {
			return (static_cast<int64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
			};

		const int64_t idle   { toInt64(idleTime)  };
		const int64_t kernel { toInt64(kernelTime)};
		const int64_t user   { toInt64(userTime)  };

		// 前回取得時からの差分を計算
		const int64_t deltaIdle{ idle - m_prevIdleTime };
		const int64_t deltaTotal{ (kernel - m_prevKernelTime) + (user - m_prevUserTime) };

		if (deltaTotal > 0)
			m_cpuUsage = 1.0f - static_cast<float>(deltaIdle) / static_cast<float>(deltaTotal);

		// 今回の値を次回の差分計算のために保存
		m_prevIdleTime   = idle;
		m_prevKernelTime = kernel;
		m_prevUserTime   = user;
	}

	void WindowsPerformanceProvider::updateMemory()
	{
		// メモリ情報を取得するための構造体を初期化
		MEMORYSTATUSEX ms{};
		ms.dwLength = sizeof(ms);
		if (!GlobalMemoryStatusEx(&ms)) return;

		m_memoryUsage = static_cast<float>(ms.dwMemoryLoad) / 100.0f; // 現在のメモリ消費率を0.0～1.0に正規化して渡す
	}

	void WindowsPerformanceProvider::updateDisk()
	{
		if (!m_hDisk) return;

		// ディスクパフォーマンス情報を取得するための構造体を初期化
		DISK_PERFORMANCE dp{};
		DWORD bytesReturned{};

		// ディスクI/O情報を取得、失敗したら早期リターン
		if (!DeviceIoControl(
			static_cast<HANDLE>(m_hDisk),
			IOCTL_DISK_PERFORMANCE,
			nullptr, 0,
			&dp, sizeof(dp),
			&bytesReturned, nullptr)) return;

		// タスクマネージャーの「アクティブな時間」と同じ計算方式：
		// activeTime = (ビジー時間) / (経過時間) = (ReadTime + WriteTime) / QueryTime
		const int64_t curReadTime  { dp.ReadTime.QuadPart  };
		const int64_t curWriteTime { dp.WriteTime.QuadPart };
		const int64_t curQueryTime { dp.QueryTime.QuadPart };

		const int64_t deltaQuery{ curQueryTime - m_prevQueryTime };
		if (deltaQuery > 0)
		{
			const int64_t deltaBusy{ (curReadTime - m_prevReadTime) + (curWriteTime - m_prevWriteTime) };
			m_diskActivity = static_cast<float>(deltaBusy) / static_cast<float>(deltaQuery);
			if (m_diskActivity > 1.0f) m_diskActivity = 1.0f;
			if (m_diskActivity < 0.0f) m_diskActivity = 0.0f;
		}

		m_prevReadTime  = curReadTime;
		m_prevWriteTime = curWriteTime;
		m_prevQueryTime = curQueryTime;
	}
}