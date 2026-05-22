#include "StringConverter.h"
#include <Windows.h>
#include <vector>

namespace platform::utility
{
	std::string StringConverter::utf8ToShiftJis(const std::string& utf8Str) const
	{
		if (utf8Str.empty())
			return "";

		// JSONから読み込んだUTF-8の日本語をDxLibに渡すには、Shift_JISに変換が必須
		// UTF-8のバイト列をそのままShift_JISとして解釈すると文字化けするため、
		// Windows APIを使用して段階的に変換する

		// UTF-8 → UTF-16（ワイド文字）
		// WindowsのAPIはUTF-16をネイティブに扱うため、まず中間形式に変換
		int wideSize{ MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0) };
		if (wideSize == 0)
			return utf8Str;

		std::vector<wchar_t> wide(wideSize);
		MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wide.data(), wideSize);

		// UTF-16 → Shift_JIS
		// DxLibの描画関数がShift_JISを期待しているため、最終的にこのエンコーディングに変換
		int sjisSize{ WideCharToMultiByte(CP_ACP, 0, wide.data(), -1, nullptr, 0, nullptr, nullptr) };
		if (sjisSize == 0)
			return utf8Str;

		std::string result(sjisSize - 1, '\0');
		WideCharToMultiByte(CP_ACP, 0, wide.data(), -1, &result[0], sjisSize, nullptr, nullptr);

		return result;
	}
}
