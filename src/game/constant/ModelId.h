#pragma once
#include <string_view>

namespace game::constant
{
    /// @brief モデルIDの定数（タイポ防止・JSON整合性維持）
    namespace model_id
    {
        constexpr std::string_view PLAYER = "player";
        constexpr std::string_view GROUND = "ground";
        constexpr std::string_view ENEMY = "enemy";
    }
}