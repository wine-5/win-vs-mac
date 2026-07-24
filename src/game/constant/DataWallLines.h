#pragma once
#include <array>
#include <string_view>

namespace game::constant
{
	/**
	 * @brief データ壁に流す文字列
	 *
	 * ターゲットがプログラマーなので、様々な言語のコード片・システムログ・
	 * 逆アセンブル・16進ダンプを混ぜて「システムの内側を覗いている」感じを出す。
	 * 侵入者としてApple側の名前を混ぜ、Mac軍に侵食されている状況も示す。
	 *
	 * 行頭の記号で色が決まる（DataWallSystem::lineColor）：
	 *   "[ok]" = 青 / "[warn]" = 黄 / "[intruder]" = 赤 / それ以外（コード片）= 淡い水色
	 */
	namespace data_wall
	{
		constexpr std::array<std::string_view, 64> LINES{
			// --- システムログ（正常） ---
			"[ok] kernel32.dll mapped 0x7ffb2c40",
			"[ok] ntfs journal flush 12.4MB/s",
			"[ok] page fault soft=1284 hard=3",
			"[ok] scheduler quantum=15ms",
			"[ok] registry hive loaded HKLM",
			"[ok] tcp socket bound :49152",
			"[ok] d3d11 device created",
			"[ok] thread pool workers=8",

			// --- システムログ（警告） ---
			"[warn] handle leak pid=4812",
			"[warn] gdi objects 9821/10000",
			"[warn] working set trim requested",
			"[warn] disk queue depth=32",
			"[warn] heap fragmentation 41%",

			// --- 侵入者（Mac軍） ---
			"[intruder] apple.process detected",
			"[intruder] safari.exe spawned",
			"[intruder] xcode build daemon found",
			"[intruder] launchd port hijack",
			"[intruder] mach_msg trap blocked",

			// --- C / C++ ---
			"if (hr != S_OK) return hr;",
			"std::unique_ptr<T> p = std::make_unique<T>();",
			"while (!queue.empty()) queue.pop();",
			"for (auto& e : entities) e.update(dt);",
			"static_assert(sizeof(void*) == 8);",
			"template <typename T> struct Handle {};",
			"co_await socket.async_read(buffer);",
			"constexpr auto kMaxEntities = 4096;",

			// --- C# ---
			"public async Task<int> RunAsync() {",
			"var query = list.Where(x => x.IsActive);",
			"using var stream = File.OpenRead(path);",

			// --- Python ---
			"def resolve(path: str) -> Path:",
			"with open(path, 'rb') as f: data = f.read()",
			"async def main(): await gather(*tasks)",
			"[x**2 for x in range(256) if x % 3 == 0]",

			// --- JavaScript / TypeScript ---
			"const res = await fetch('/api/status');",
			"export type Result<T> = Ok<T> | Err;",
			"array.reduce((a, b) => a + b, 0);",

			// --- Rust ---
			"let mut buf = Vec::with_capacity(1024);",
			"impl Drop for Guard { fn drop(&mut self) }",
			"match result { Ok(v) => v, Err(e) => panic!() }",
			"unsafe { ptr::write_volatile(dst, val) }",

			// --- Go ---
			"go func() { ch <- process(job) }()",
			"defer mu.Unlock()",
			"if err != nil { return nil, err }",

			// --- Java / Kotlin ---
			"synchronized (lock) { queue.notifyAll(); }",
			"val flow = MutableStateFlow(State.Idle)",

			// --- SQL ---
			"SELECT pid, name FROM processes WHERE cpu > 80;",
			"UPDATE handles SET closed = 1 WHERE leaked;",

			// --- シェル / PowerShell ---
			"$ tasklist /fi \"imagename eq safari.exe\"",
			"Get-Process | Sort-Object CPU -Descending",
			"$ netstat -ano | findstr ESTABLISHED",

			// --- アセンブリ ---
			"mov rax, qword ptr [rsp+28h]",
			"lock cmpxchg [rbx], rcx",
			"call QueryPerformanceCounter",
			"test eax, eax",
			"jne short loc_140001A20",
			"ret",

			// --- 低レベル / ダンプ ---
			"0x4d 0x5a 0x90 0x00 0x03 0x00 0x00",
			"0xde 0xad 0xbe 0xef 0xca 0xfe",
			"thread 0x1a4 state=WAITING",
			"heap block 0x00007ff6 size=4096",
			"IRQL_NOT_LESS_OR_EQUAL 0x0000000A",
			"virtual address 0x00007ff6b2c40000",
			"CR3 = 0x00000000001ab000",
			"stack trace depth 17 frames",
		};
	} // namespace data_wall
} // namespace game::constant
