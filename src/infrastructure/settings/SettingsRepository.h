#pragma once
#include "core/interface/ISettingsRepository.h"

namespace infrastructure::settings
{
	/**
	 * @brief 設定を JSON ファイルへ読み書きするクラス
	 *
	 * 保存先は exe と同じ場所の settings.json にしている。ZIP で配って解凍先から
	 * 起動される想定のため、%APPDATA% へ置くよりも持ち運びが確実で、
	 * 遊び終えた人のPCに設定ファイルが残らない
	 */
	class SettingsRepository : public core::iface::ISettingsRepository
	{
	  public:
		/**
		 * @brief settings.json から設定を読み込む
		 *
		 * ファイルが無い（初回起動）、壊れている、値が範囲外、のいずれでも例外は投げず
		 * 既定値へ倒す。設定が読めないことは遊べないことを意味しないため
		 * @return 読み込んだ設定
		 */
		[[nodiscard]] core::data::GameSettings load() const override;

		/**
		 * @brief settings.json へ設定を書き出す
		 *
		 * 書き込みに失敗しても例外は投げない（読み取り専用の場所へ置かれた場合など）。
		 * その回の変更が次回に残らないだけで、遊ぶこと自体は妨げない
		 * @param settings 保存する設定
		 */
		void save(const core::data::GameSettings& settings) const override;

	  private:
		/** @brief 保存先のパス（カレントディレクトリ＝exeの場所からの相対） */
		static constexpr const char* FILE_PATH{ "settings.json" };
	};
} // namespace infrastructure::settings
