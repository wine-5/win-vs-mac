#pragma once
#include <string_view>

namespace core::data
{
	/**
	 * @brief ゲームの難易度
	 *
	 * セレクト画面の難易度ウィンドウで選び、GameManager がシーンをまたいで保持する。
	 * 敵のパラメータ（各敵JSONの hard ブロック）と演出の切り替えに使う。
	 */
	enum class Difficulty
	{
		Normal,
		Hard
	};

	/// @brief 難易度ウィンドウ（WebView）とやり取りする文字列
	inline constexpr std::string_view DIFFICULTY_TEXT_NORMAL{ "NORMAL" };
	inline constexpr std::string_view DIFFICULTY_TEXT_HARD{ "HARD" };

	/**
	 * @brief 難易度の文字列を列挙へ変換する
	 * @param text 難易度文字列（"NORMAL" | "HARD"）
	 * @return 対応する Difficulty。未知の文字列は Normal を返す
	 */
	[[nodiscard]] inline constexpr Difficulty toDifficulty(std::string_view text) noexcept
	{
		if (text == DIFFICULTY_TEXT_HARD)
			return Difficulty::Hard;
		return Difficulty::Normal;
	}

	/**
	 * @brief 難易度を表示用の文字列へ変換する
	 * @param difficulty 難易度
	 * @return "NORMAL" または "HARD"
	 */
	[[nodiscard]] inline constexpr std::string_view toText(Difficulty difficulty) noexcept
	{
		if (difficulty == Difficulty::Hard)
			return DIFFICULTY_TEXT_HARD;
		return DIFFICULTY_TEXT_NORMAL;
	}
} // namespace core::data
