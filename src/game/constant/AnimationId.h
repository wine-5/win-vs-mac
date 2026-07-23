#pragma once
#include <string_view>

namespace game::constant
{
    /// @brief アニメーションIDの定数（resources.json の animations に登録されたID）
    namespace animation_id
    {
        // Player
        constexpr std::string_view PLAYER_IDLE  = "anim_player_idle";
        constexpr std::string_view PLAYER_WALK  = "anim_player_walk";
        constexpr std::string_view PLAYER_RUN   = "anim_player_run";
        constexpr std::string_view PLAYER_SLASH = "anim_player_slash";
        constexpr std::string_view PLAYER_SPIN  = "anim_player_spin";
        constexpr std::string_view PLAYER_HIT   = "anim_player_hit";
        constexpr std::string_view PLAYER_DYING = "anim_player_dying";
        constexpr std::string_view PLAYER_JUMP  = "anim_player_jump";

		// 敵（Xcode/Mac）のアニメIDは各enemy JSONの animations 配列に文字列で持つため、
		// ここには定数を置かない（Playerのみコード内でIDを参照する）
	} // namespace animation_id
} // namespace game::constant
