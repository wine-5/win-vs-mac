#pragma once
#include <string_view>

namespace game::constant::prop_role
{
	/**
	 * @brief 配置物の特別な役割（stageCatalog.json の role に対応）
	 *
	 * 種類（id）ではなく役割で判定することで、見た目違いの扉を何種類でも作れる。
	 * roleが空の配置物は普通の床・壁として扱う。
	 */

	// ボス出現で閉じ、入り口を塞ぐ扉
	constexpr std::string_view BOSS_GATE = "bossGate";
} // namespace game::constant::prop_role
