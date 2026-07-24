#pragma once
#include <array>
#include "core/data/FileExtensionBonus.h"
#include "core/data/FileExtensionType.h"

namespace infrastructure::resource::repository
{
	/**
	 * @brief 拡張子種別ごとのパラメータボーナスを保持するリポジトリ
	 *
	 * extensionBonus.json からボーナス値を読み込む。
	 * バランス調整で頻繁に触る値のため、ソースの定数ではなくデータで持つ。
	 */
	class ExtensionBonusRepository
	{
	  public:
		/**
		 * @brief コンストラクト時に extensionBonus.json を読み込む
		 */
		ExtensionBonusRepository();

		/**
		 * @brief 拡張子種別に対応するボーナスを取得する
		 * @param type ファイル拡張子グループ種別
		 * @return 対応するボーナス値（未定義の種別は全て0）
		 */
		[[nodiscard]] const core::data::FileExtensionBonus& getBonus(
		    core::data::FileExtensionType type) const noexcept;

	  private:
		/// @brief FileExtensionType の値数（Executable〜Unknownの6種）
		static constexpr std::size_t TYPE_COUNT{ 6 };

		// FileExtensionType の値を添字にして引く
		std::array<core::data::FileExtensionBonus, TYPE_COUNT> m_bonuses{};
	};
} // namespace infrastructure::resource::repository
