#pragma once
#include <string_view>

namespace game::constant
{
    /// @brief モデルIDの定数（タイポ防止・JSON整合性維持）
    namespace model_id
    {
        constexpr std::string_view PLAYER = "player";
        constexpr std::string_view GROUND = "ground";
        constexpr std::string_view ENEMY_XCODE = "enemy_xcode";
        constexpr std::string_view ENEMY_SAFARI = "enemy_safari";
        constexpr std::string_view ENEMY_MAC = "enemy_mac";
	} // namespace model_id
} // namespace game::constant