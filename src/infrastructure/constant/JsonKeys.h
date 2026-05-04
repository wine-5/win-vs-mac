#pragma once

namespace infrastructure::constant
{
    /// @brief JSONのキー名定数（タイポ防止）
    namespace json_keys
    {
        // トップレベル
        constexpr const char* ID = "id";
        constexpr const char* CATEGORY = "category";
        constexpr const char* MODEL = "model";
        constexpr const char* COLLIDER = "collider";
        constexpr const char* TRANSFORM = "transform";
        constexpr const char* ANIMATIONS = "animations";
        constexpr const char* GAMEPLAY = "gameplay";

        // model配下
        constexpr const char* PATH = "path";
        constexpr const char* SCALE = "scale";

        // collider配下
        constexpr const char* SIZE = "size";
        constexpr const char* OFFSET = "offset";

        // animations配下
        constexpr const char* IDLE = "idle";
        constexpr const char* WALK = "walk";

        // gameplay配下
        constexpr const char* MOVE_SPEED      = "moveSpeed";
        constexpr const char* DETECTION_RANGE  = "detectionRange";
        constexpr const char* ATTACK_RANGE     = "attackRange";
        constexpr const char* MAX_HP           = "maxHp";
        constexpr const char* DEFENCE          = "defence";
        constexpr const char* ATTACK_POWER     = "attackPower";
        constexpr const char* ATTACK_COOLDOWN  = "attackCooldown";
    }
}