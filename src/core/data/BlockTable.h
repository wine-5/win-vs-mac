#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace core::data
{
	/**
	 * @brief 破壊可能ブロックの抽選表の1行
	 */
	struct BlockTableEntry
	{
		std::string m_type{};   // 実際に配置する種類ID（例: "block_ext_image"）
		float m_weight{ 0.0f }; // 相対的な出やすさ。0なら出現しない
	};

	/**
	 * @brief 破壊可能ブロックの抽選表（stageBalance.jsonのblockTable）
	 *
	 * ステージ配置に置かれた汎用ブロックを、実際の種類へ置き換えるときに使う。
	 * 重みは相対値として扱うため、合計を1に揃える必要はない。
	 * 1種類の重みを変えても他の行を直さなくてよい、という調整のしやすさを優先している。
	 */
	struct BlockTable
	{
		std::vector<BlockTableEntry> m_entries{};

		/**
		 * @brief 重みの合計を返す
		 * @return 全エントリの重みの合計（抽選可能な行が無ければ0）
		 */
		[[nodiscard]] float totalWeight() const noexcept
		{
			float total{ 0.0f };
			for (const auto& entry : m_entries)
			{
				if (entry.m_weight > 0.0f)
					total += entry.m_weight;
			}
			return total;
		}

		/**
		 * @brief 0〜totalWeight() の値から対応する種類IDを引く
		 *
		 * 乱数生成をこの構造体へ持ち込まないために、抽選値は呼び出し側から受け取る。
		 * こうするとテストで任意の値を入れて結果を確認できる。
		 * @param roll 抽選値（0以上 totalWeight() 未満）
		 * @return 種類ID。抽選可能な行が無ければ空文字列
		 */
		[[nodiscard]] std::string_view pick(float roll) const noexcept
		{
			for (const auto& entry : m_entries)
			{
				if (entry.m_weight <= 0.0f)
					continue;
				roll -= entry.m_weight;
				if (roll < 0.0f)
					return entry.m_type;
			}

			// 浮動小数の誤差で末尾を踏み抜いた場合の保険。最後の有効な行を返す
			for (auto it{ m_entries.rbegin() }; it != m_entries.rend(); ++it)
			{
				if (it->m_weight > 0.0f)
					return it->m_type;
			}
			return {};
		}
	};
} // namespace core::data
