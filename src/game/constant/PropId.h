#pragma once
#include <string_view>

namespace game::constant
{
	/// @brief 配置物の種類IDの定数（stageCatalog.json との整合を保つ）
	namespace prop_id
	{
		// 実行時にテクスチャを差し替えてシステム情報を映す壁
		constexpr std::string_view WALL_DATA = "wall_data";

		// 種類を固定せず、生成時にblockTableの重み付き抽選で中身が決まるブロック。
		// ステージ側はここに「壊せるブロックがある」ことだけを置き、
		// 何が出るかはプレイのたびに変わる
		constexpr std::string_view BLOCK_RANDOM = "block_random";
	} // namespace prop_id
} // namespace game::constant
