#pragma once
#include <string_view>

namespace game::constant
{
	/**
	 * @brief 配置物の当たり判定の役割（stageCatalog.json の collider に対応）
	 */
	enum class PropCollision
	{
		None,  // 判定なし（装飾のみ）
		Box,   // 塞ぐ障害物。AABBで押し返す（壁・柱・ファイルブロック）
		Ground // 歩ける面。傾きを考慮して足を乗せる（床・通路・坂）
	};

	/**
	 * @brief カタログの collider 文字列を PropCollision へ変換する
	 * @param collider "box" | "ground" | それ以外
	 * @return 対応する PropCollision（未知の値は None）
	 */
	constexpr PropCollision toPropCollision(std::string_view collider) noexcept
	{
		if (collider == "box")
			return PropCollision::Box;
		if (collider == "ground")
			return PropCollision::Ground;
		return PropCollision::None;
	}
} // namespace game::constant
