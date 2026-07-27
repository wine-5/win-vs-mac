#pragma once
#include <cstddef>
#include <string_view>

namespace core::iface
{
	/**
	 * @brief メモリ使用量の計測の純粋仮想クラス
	 *
	 * Game層・Infrastructure層がPlatform層（Windows API）に直接依存しないための抽象化。
	 * 計測したい処理を probe() で挟めば、その処理が確保した量が増分として記録される。
	 *
	 * 呼ぶときは core/utility/Probe.h の core::probe::mark / note を使う。
	 * このヘッダは実装への依存を持たない純粋な宣言に保つこと。
	 */
	class IMemoryProbe
	{
	  public:
		virtual ~IMemoryProbe() = default;

		/**
		 * @brief 計測地点を記録する（直前の地点からの増分を併記する）
		 * @param label 計測地点の名前（どの処理の直後かが分かる文字列）
		 */
		virtual void mark(std::string_view label) = 0;

		/**
		 * @brief 計測ログへ任意の1行を追記する（メモリ量を伴わない補足情報用）
		 * @param text 追記する行
		 */
		virtual void note(std::string_view text) = 0;

		/**
		 * @brief 計測ログを空にして計測を開始する（起動直後に一度だけ呼ぶ）
		 */
		virtual void reset() = 0;

		/**
		 * @brief 現在のプロセスのプライベートバイトを取得する
		 * @return プライベートバイト数（取得に失敗した場合は 0）
		 */
		[[nodiscard]] virtual std::size_t getPrivateBytes() const = 0;

		/**
		 * @brief 現在のプロセスのワーキングセットを取得する
		 * @return ワーキングセット数（取得に失敗した場合は 0）
		 */
		[[nodiscard]] virtual std::size_t getWorkingSet() const = 0;
	};
} // namespace core::iface
