#pragma once
#include "core/data/FileExtensionBonus.h"
#include "core/data/FileExtensionType.h"
#include "core/utility/MathConstants.h"
#include <string>
#include <string_view>

namespace game::utility
{
	/**
	 * @brief 拡張子種別が伸ばす能力の表記を扱うクラス
	 *
	 * 装備スロット（右下HUD）とインベントリが同じ表記を使う。
	 * 対応表が別々にあると、種別を足したときに片方だけ古い表記のまま残る。
	 *
	 * @note 表記はASCIIに限る。Shift_JIS変換を通さずに描いているViewがあるため、
	 *       日本語を混ぜるとそちらで文字化けする
	 */
	class ExtensionBonusLabel
	{
	  public:
		/**
		 * @brief 種別が伸ばす能力の短い名前を返す
		 *
		 * B.は弾（Window弾）のこと。SPD+（移動速度）・RNG+（攻撃範囲）と
		 * 紛れないよう区別している
		 * @param type 拡張子種別
		 * @return 能力の短い名前（例: "ATK+"）
		 */
		[[nodiscard]] static constexpr std::string_view toLabel(core::data::FileExtensionType type) noexcept
		{
			switch (type)
			{
			case core::data::FileExtensionType::Executable: return "ATK+";
			case core::data::FileExtensionType::Document: return "SPD+";
			case core::data::FileExtensionType::Image: return "DEF+";
			case core::data::FileExtensionType::Audio: return "HP+";
			case core::data::FileExtensionType::SourceCode: return "CRIT+";
			case core::data::FileExtensionType::Shortcut: return "B.SPD+";
			case core::data::FileExtensionType::Video: return "B.RNG+";
			case core::data::FileExtensionType::Archive: return "ALL+";
			default: return "RNG+";
			}
		}

		/**
		 * @brief 種別が伸ばす能力の上昇量を返す
		 *
		 * アーカイブは全項目を少しずつ上げるため、代表となる1つの値を選べない。
		 * その場合は0を返し、呼び出し側は数値を出さずに "ALL+" だけを見せる
		 * @param type 拡張子種別
		 * @param bonus 種別に対応するボーナス値（extensionBonus.json 由来）
		 * @return 上昇量。代表値を選べない種別は0
		 */
		[[nodiscard]] static constexpr float toAmount(core::data::FileExtensionType type,
		    const core::data::FileExtensionBonus& bonus) noexcept
		{
			switch (type)
			{
			case core::data::FileExtensionType::Executable: return bonus.atk;
			case core::data::FileExtensionType::Document: return bonus.spd;
			case core::data::FileExtensionType::Image: return bonus.def;
			case core::data::FileExtensionType::Audio: return bonus.hp;
			// クリティカル率は確率なので、表示は百分率へ直す
			case core::data::FileExtensionType::SourceCode: return bonus.criticalRate * core::utility::RATIO_TO_PERCENT;
			case core::data::FileExtensionType::Shortcut: return bonus.projectileSpeed;
			case core::data::FileExtensionType::Video: return bonus.projectileRange;
			case core::data::FileExtensionType::Archive: return 0.0f;
			default: return bonus.attackRange;
			}
		}

		/**
		 * @brief 「能力名＋上昇量」の表記を組み立てる
		 * @param type 拡張子種別
		 * @param bonus 種別に対応するボーナス値
		 * @return 表示する文字列（例: "DEF+3" / "CRIT+8%" / "ALL+"）
		 */
		[[nodiscard]] static std::string format(core::data::FileExtensionType type,
		    const core::data::FileExtensionBonus& bonus)
		{
			std::string text{ toLabel(type) };

			const float amount{ toAmount(type, bonus) };
			if (amount <= 0.0f)
				return text;

			text += std::to_string(static_cast<int>(amount));
			if (type == core::data::FileExtensionType::SourceCode)
				text += "%";
			return text;
		}
	};
} // namespace game::utility
