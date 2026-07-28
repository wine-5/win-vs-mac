#pragma once
#include <cstddef>
#include <string_view>
#include "core/interface/IMemoryProbe.h"

namespace platform::diagnostics
{
	/**
	 * @brief IMemoryProbe の Windows 実装
	 *
	 * プロセスのプライベートバイトを任意の地点で記録し、直前の地点からの増分と共に
	 * memory_probe.txt へ追記する。どのサブシステムが何MB確保しているかの切り分けに使う。
	 *
	 * Windows API（psapi）はこのクラスの実装ファイル側に閉じ込める。
	 * ヘッダに windows.h を出すと、取り込んだ側の std::max などがマクロで壊れるため。
	 */
	class MemoryProbe : public core::iface::IMemoryProbe
	{
	  public:
		MemoryProbe() = default;

		/**
		 * @brief 計測地点を記録する（直前の地点からの増分を併記する）
		 * @param label 計測地点の名前
		 */
		void mark(std::string_view label) override;

		/**
		 * @brief 計測ログへ任意の1行を追記する
		 * @param text 追記する行
		 */
		void note(std::string_view text) override;

		/**
		 * @brief 計測ログを空にして計測を開始する
		 */
		void reset() override;

		/**
		 * @brief 現在のプロセスのプライベートバイトを取得する
		 * @return プライベートバイト数（取得に失敗した場合は 0）
		 */
		[[nodiscard]] std::size_t getPrivateBytes() const override;

		/**
		 * @brief 現在のプロセスのワーキングセットを取得する
		 * @return ワーキングセット数（取得に失敗した場合は 0）
		 */
		[[nodiscard]] std::size_t getWorkingSet() const override;

	  private:
		// 直前の計測地点でのプライベートバイト（増分計算用）
		std::size_t m_lastPrivateBytes{ 0 };
	};
} // namespace platform::diagnostics
