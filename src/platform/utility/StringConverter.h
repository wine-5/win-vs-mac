#pragma once
#include "core/interface/IStringConverter.h"

namespace platform::utility
{
	/**
	 * @brief 文字列エンコーディング変換の実装（Windows API使用）
	 */
	class StringConverter : public core::iface::IStringConverter
	{
	public:
		/**
		 * @brief UTF-8 を Shift_JIS に変換する
		 * JSONファイルはUTF-8で保存されているが、DxLibの描画関数はShift_JISを期待しているため、変換が必要
		 * @param utf8Str UTF-8 の文字列
		 * @return Shift_JIS の文字列
		 */
		std::string utf8ToShiftJis(const std::string& utf8Str) const override;

		/**
		 * @brief UTF-8 を Unicode (ワイド文字) に変換する
		 * @param utf8Str UTF-8 の文字列
		 * @return Unicode の文字列
		 */
		std::wstring utf8ToWide(const std::string& utf8Str) const override;
	};
}
