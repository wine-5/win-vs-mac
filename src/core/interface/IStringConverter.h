#pragma once
#include <string>

namespace core::iface
{
	/**
	 * @brief 文字列エンコーディング変換のインターフェース
	 */
	class IStringConverter
	{
	public:
		virtual ~IStringConverter() = default;

		/**
		 * @brief UTF-8 を Shift_JIS に変換する
		 * @param utf8Str UTF-8 の文字列
		 * JSONファイルはUTF-8で保存されているが、DxLibの描画関数はShift_JISを期待しているため、変換が必要
		 * @return Shift_JIS の文字列
		 */
		virtual std::string utf8ToShiftJis(const std::string& utf8Str) const = 0;

		/**
		 * @brief UTF-8 を Unicode (ワイド文字) に変換する
		 * @param utf8Str UTF-8 の文字列
		 * @return Unicode の文字列
		 */
		virtual std::wstring utf8ToWide(const std::string& utf8Str) const = 0;
	};
}
