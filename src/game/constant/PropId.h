#pragma once
#include <string_view>

namespace game::constant
{
	/// @brief 配置物の種類IDの定数（stageCatalog.json との整合を保つ）
	namespace prop_id
	{
		// 実行時にテクスチャを差し替えてシステム情報を映す壁
		constexpr std::string_view WALL_DATA = "wall_data";
	} // namespace prop_id
} // namespace game::constant
