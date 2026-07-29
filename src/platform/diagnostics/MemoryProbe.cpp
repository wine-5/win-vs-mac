#include "platform/diagnostics/MemoryProbe.h"
#include "core/constant/DebugFlags.h"

// windows.h は max / min / DrawText などをマクロで定義する。取り込む前に塞いでおく
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <psapi.h>
#include <cstdio>
#include <string_view>

#pragma comment(lib, "psapi.lib")

namespace
{
	constexpr double BYTES_PER_MB{ 1024.0 * 1024.0 };
	constexpr const char* PROBE_LOG_PATH{ "memory_probe.txt" };

	/**
	 * @brief プロセスのメモリ情報を取得する
	 * @param counters 取得先
	 * @return 取得できたか
	 */
	bool queryCounters(PROCESS_MEMORY_COUNTERS_EX& counters)
	{
		counters.cb = sizeof(counters);
		return GetProcessMemoryInfo(GetCurrentProcess(),
		           reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters),
		           sizeof(counters)) != 0;
	}

	/**
	 * @brief 計測ログへ1行追記する
	 * @param line 追記する行（改行は呼び出し側で含めない）
	 */
	void appendLine(std::string_view line)
	{
		if (!core::constant::WRITE_DEBUG_LOG_FILES)
			return;

		std::FILE* file{ nullptr };
		if (fopen_s(&file, PROBE_LOG_PATH, "a") != 0 || file == nullptr)
			return;

		// string_view は終端が保証されないため、長さを指定して書き出す
		std::fprintf(file, "%.*s\n", static_cast<int>(line.size()), line.data());
		std::fclose(file);
	}
} // namespace

namespace platform::diagnostics
{
	std::size_t MemoryProbe::getPrivateBytes() const
	{
		PROCESS_MEMORY_COUNTERS_EX counters{};
		if (!queryCounters(counters))
			return 0;
		return counters.PrivateUsage;
	}

	std::size_t MemoryProbe::getWorkingSet() const
	{
		PROCESS_MEMORY_COUNTERS_EX counters{};
		if (!queryCounters(counters))
			return 0;
		return counters.WorkingSetSize;
	}

	void MemoryProbe::mark(std::string_view label)
	{
		const std::size_t privateBytes{ getPrivateBytes() };
		const double deltaMb{ (static_cast<double>(privateBytes) -
			                      static_cast<double>(m_lastPrivateBytes)) /
			                  BYTES_PER_MB };
		m_lastPrivateBytes = privateBytes;

		char line[256]{};
		std::snprintf(line, sizeof(line), "%-40.*s private=%8.1fMB  delta=%+8.1fMB  ws=%8.1fMB",
		    static_cast<int>(label.size()), label.data(),
		    static_cast<double>(privateBytes) / BYTES_PER_MB,
		    deltaMb,
		    static_cast<double>(getWorkingSet()) / BYTES_PER_MB);

		appendLine(line);
	}

	void MemoryProbe::note(std::string_view text)
	{
		appendLine(text);
	}

	void MemoryProbe::reset()
	{
		if (core::constant::WRITE_DEBUG_LOG_FILES)
		{
			std::FILE* file{ nullptr };
			if (fopen_s(&file, PROBE_LOG_PATH, "w") == 0 && file != nullptr)
				std::fclose(file);
		}

		m_lastPrivateBytes = getPrivateBytes();
	}
} // namespace platform::diagnostics
