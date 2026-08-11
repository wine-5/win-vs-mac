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
	 * m_maxEquipped 個だけで、それ以降は「持っているが挿していない」状態になる。
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
		/// @brief InGame中に能力へ乗せられる個数の初期値（セレクト画面の3つと合わせて最大6）
		static constexpr int DEFAULT_MAX_EQUIPPED{ 3 };

		// 拾った順に並ぶ。先頭 m_maxEquipped 個が装備中
		std::vector<core::data::FileExtensionType> m_acquired{};

		// 能力へ乗せられる個数。RAMブロックを壊すと道中で増えるため、
		// 定数ではなく持ち主ごとの値として持つ
		int m_maxEquipped{ DEFAULT_MAX_EQUIPPED };

		// 装備中の拡張子の効果へ掛かる倍率。隔離フォルダの当たりを引くと上がる。
		// 効果を足し引きするのは拡張子を挿し外しした瞬間だけなので、
		// 倍率もその都度この値を見て掛ける。あとから素の値を計算し直す仕組みは無い
		float m_bonusMultiplier{ 1.0f };

		/**
		 * @brief 装備中（効果が乗っている）の個数を返す
		 * @return 装備中の個数（0〜m_maxEquipped）
		 */
		[[nodiscard]] int equippedCount() const noexcept
		{
			return std::min(static_cast<int>(m_acquired.size()), m_maxEquipped);
		}

		/**
		 * @brief 指定の位置が装備中かを返す
		 * @param index m_acquired 上の位置
		 * @return 装備中ならtrue
		 */
		[[nodiscard]] bool isEquipped(int index) const noexcept
		{
			return index >= 0 && index < m_maxEquipped;
		}
	};
} // namespace game::component::combat
