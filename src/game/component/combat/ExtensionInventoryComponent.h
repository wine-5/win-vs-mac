#pragma once
#include "core/data/FileExtensionType.h"
#include <algorithm>
#include <vector>

namespace game::component::combat
{
	/**
	 * @brief 道中で拾った拡張子を持つコンポーネント（プレイヤーに付く）
	 *
	 * 拾ったものはすべてここへ入る。ただし能力に効果が乗るのは先頭の
	 * MAX_EQUIPPED 個だけで、それ以降は「持っているが挿していない」状態になる。
	 *
	 * 拾った瞬間に捨てると、何を捨てて何を挿すかという選択が生まれない。
	 * 全部持たせたうえで挿せる数を絞ることで、リネームブロックの前で
	 * 「どれと入れ替えるか」を考える時間が生まれる。
	 *
	 * @note セレクト画面で選んだファイル（持ち込み）は FileEquipmentData が持つ。
	 *       出どころが違うものを1つの入れ物へ混ぜると、リザルトで
	 *       「何を持ち込んで何を拾ったか」を分けて見せられなくなる
	 */
	struct ExtensionInventoryComponent
	{
		/// @brief InGame中に能力へ乗せられる個数（セレクト画面の3つと合わせて最大6）
		static constexpr int MAX_EQUIPPED{ 3 };

		// 拾った順に並ぶ。先頭 MAX_EQUIPPED 個が装備中
		std::vector<core::data::FileExtensionType> m_acquired{};

		/**
		 * @brief 装備中（効果が乗っている）の個数を返す
		 * @return 装備中の個数（0〜MAX_EQUIPPED）
		 */
		[[nodiscard]] int equippedCount() const noexcept
		{
			return std::min(static_cast<int>(m_acquired.size()), MAX_EQUIPPED);
		}

		/**
		 * @brief 指定の位置が装備中かを返す
		 * @param index m_acquired 上の位置
		 * @return 装備中ならtrue
		 */
		[[nodiscard]] bool isEquipped(int index) const noexcept
		{
			return index >= 0 && index < MAX_EQUIPPED;
		}
	};
} // namespace game::component::combat
