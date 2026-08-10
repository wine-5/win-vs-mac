#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include <array>

namespace game::utility
{
	/// @brief 画面に並べる能力値の項目数
	inline constexpr int PLAYER_STAT_COUNT{ 8 };

	// 能力値の並び。左下HUD・インベントリ・セレクト画面のすべてで同じ順に見せる。
	// アイコン・ラベル・値を別々の配列で持つため、位置は名前で参照して食い違いを防ぐ
	inline constexpr int STAT_INDEX_HP{ 0 };
	inline constexpr int STAT_INDEX_ATK{ 1 };
	inline constexpr int STAT_INDEX_DEF{ 2 };
	inline constexpr int STAT_INDEX_SPD{ 3 };
	inline constexpr int STAT_INDEX_RNG{ 4 };
	/// @brief クリティカル率だけは割合なので百分率で持つ。この位置だけ書式が変わる
	inline constexpr int STAT_INDEX_CRIT{ 5 };
	inline constexpr int STAT_INDEX_BSPD{ 6 };
	inline constexpr int STAT_INDEX_BRNG{ 7 };

	/// @brief 能力値をまとめて並べた表
	using PlayerStatValues = std::array<float, PLAYER_STAT_COUNT>;

	/**
	 * @brief プレイヤーの現在の能力値を集める
	 *
	 * 値の持ち主は種類ごとに違う（攻撃はAttack、HPはHealth、速度はStats）。
	 * どの画面も同じ並びで見せるため、集める処理を1か所に置く
	 * @param componentManager ComponentManagerの参照
	 * @param playerId プレイヤーのEntityID
	 * @return 現在値の表（読めなかった項目は0のまま）
	 */
	[[nodiscard]] PlayerStatValues collectPlayerStats(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId playerId);

	/**
	 * @brief プレイヤーの強化前（素）の能力値を集める
	 *
	 * 現在値と突き合わせて「どれだけ上がっているか」を出すための基準点。
	 * 強化の出どころ（装備ファイル・拾った拡張子・将来のバフ）は問わない
	 * @param componentManager ComponentManagerの参照
	 * @param playerId プレイヤーのEntityID
	 * @return 素の値の表（控えが無ければ全項目0＝強化なし扱い）
	 */
	[[nodiscard]] PlayerStatValues collectPlayerBaseStats(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId playerId);
} // namespace game::utility
