#pragma once
#include <string_view>

namespace game::constant
{
    /// @brief アニメーションIDの定数（rawModels に登録されたアニメーションリソース）
    namespace animation_id
    {
        constexpr std::string_view PLAYER_IDLE = "anim_player_idle";
        constexpr std::string_view PLAYER_WALK = "anim_player_walk";
    }
}
