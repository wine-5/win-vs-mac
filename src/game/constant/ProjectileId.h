#pragma once
#include <string_view>

namespace game::constant
{
	/// @brief 弾IDの定数（タイポ防止・JSON整合性維持）
	namespace projectile_id
	{
		constexpr std::string_view PLAYER_WINDOW = "player_window";
		constexpr std::string_view ENEMY_SAFARI_TAB = "enemy_safari_tab";
	} // namespace projectile_id
} // namespace game::constant
