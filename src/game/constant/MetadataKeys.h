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
		constexpr std::string_view DASH_MULTIPLIER = "dashMultiplier";
		constexpr std::string_view DETECTION_RANGE = "detectionRange";
		constexpr std::string_view ATTACK_RANGE = "attackRange";
		constexpr std::string_view MAX_HP = "maxHp";
		constexpr std::string_view DEFENCE = "defence";
		constexpr std::string_view ATTACK_POWER = "attackPower";
		constexpr std::string_view ATTACK_COOLDOWN = "attackCooldown";
		constexpr std::string_view HOVER_HEIGHT = "hoverHeight";
		constexpr std::string_view PREFERRED_DISTANCE_MIN = "preferredDistanceMin";
		constexpr std::string_view PREFERRED_DISTANCE_MAX = "preferredDistanceMax";
		constexpr std::string_view FIRE_COOLDOWN = "fireCooldown";
		constexpr std::string_view FACING_YAW_OFFSET = "facingYawOffset";
	} // namespace metadata_keys
} // namespace game::constant