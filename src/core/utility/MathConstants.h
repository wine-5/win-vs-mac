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
} // namespace core::utility
