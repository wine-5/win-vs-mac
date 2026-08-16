#pragma once
#include "core/data/GameSettings.h"

namespace core::iface
{
	/**
	 * @brief 設定値の保存・読み込みを行うインターフェース
	 *
	 * Game 層が保存先（ファイルなのかレジストリなのか）を知らずに済むようにする。
	 * リソース読み込みと違い、失敗しても例外を投げない。設定が読めないことは
	 * 遊べないことを意味しないため、既定値で起動を続ける
	 */
	class ISettingsRepository
	{
	  public:
		virtual ~ISettingsRepository() = default;

		/**
		 * @brief 保存された設定を読み込む
		 * @return 読み込んだ設定。保存が無い・壊れている場合は既定値
		 */
		[[nodiscard]] virtual core::data::GameSettings load() const = 0;

		/**
		 * @brief 設定を保存する
		 * @param settings 保存する設定
		 */
		virtual void save(const core::data::GameSettings& settings) const = 0;
	};
} // namespace core::iface
