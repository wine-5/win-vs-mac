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

        // Mac（ボス）
        constexpr std::string_view MAC_IDLE         = "anim_mac_idle";
        constexpr std::string_view MAC_WALK         = "anim_mac_walk";
        constexpr std::string_view MAC_RUN          = "anim_mac_run";
        constexpr std::string_view MAC_SWING_ATTACK = "anim_mac_swing_attack";
        constexpr std::string_view MAC_MAGIC_ATTACK = "anim_mac_magic_attack";
        constexpr std::string_view MAC_DYING        = "anim_mac_dying";

        // Xcode（雑魚）
        constexpr std::string_view XCODE_IDLE        = "anim_xcode_idle";
        constexpr std::string_view XCODE_WALK        = "anim_xcode_walk";
        constexpr std::string_view XCODE_GROUND_SLAM = "anim_xcode_ground_slam";
        constexpr std::string_view XCODE_DYING       = "anim_xcode_dying";
	} // namespace animation_id
} // namespace game::constant
