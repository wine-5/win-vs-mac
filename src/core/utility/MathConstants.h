#pragma once
#include <numbers>

namespace core::utility
{
	/// @brief 円周率
	constexpr float PI{ std::numbers::pi_v<float> };

	/// @brief 全周（2π）。角度の一様乱数や円の分割に使う
	constexpr float TWO_PI{ 2.0f * PI };

	/// @brief 度をラジアンへ変換する係数（JSONは度で持ち、内部計算はラジアンで行う）
	constexpr float DEG_TO_RAD{ PI / 180.0f };

	/// @brief ラジアンを度へ変換する係数（度で受け取るAPIへ計算結果を渡すときに使う）
	constexpr float RAD_TO_DEG{ 180.0f / PI };

	/// @brief 割合（0.0〜1.0）を百分率へ変換する係数
	///
	/// クリティカル率やCPU使用率のように、内部では割合で持ち表示だけ百分率にする値で使う。
	/// 生の100を書くと「なぜ100倍しているのか」が読み取れず、単位の変換だと分からない
	constexpr float RATIO_TO_PERCENT{ 100.0f };
} // namespace core::utility
