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
		// Safariが投げるブラウザタブ弾（3種ランダム）
		constexpr std::string_view TAB_STORAGE_FULL = "tab_storage_full";
		constexpr std::string_view TAB_SAFARI_ERROR = "tab_safari_error";
		constexpr std::string_view TAB_XCODE_BUILDING = "tab_xcode_building";
		// Boss（Mac）が投げる虹色くるくる
		constexpr std::string_view BOSS_RAINBOW_WHEEL = "boss_rainbow_wheel";
	} // namespace model_id
} // namespace game::constant