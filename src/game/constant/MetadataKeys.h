#pragma once
#include <string_view>

namespace game::constant
{
	/// @brief JSONメタデータのプロパティキーの定数（タイポ防止・JSON整合性維持）
	namespace metadata_keys
	{
		constexpr std::string_view IDLE_ANIM = "idleAnim";
		constexpr std::string_view WALK_ANIM = "walkAnim";
		constexpr std::string_view MOVE_SPEED = "moveSpeed";
		constexpr std::string_view DETECTION_RANGE = "detectionRange";
	}
}