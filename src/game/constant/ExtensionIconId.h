#pragma once
#include <string_view>
#include "core/data/FileExtensionType.h"

namespace game::constant
{
	/// @brief 拡張子アイコンの画像ID（resources.json との整合を保つ）
	namespace extension_icon_id
	{
		/// @brief 空きスロットに出す「何も入っていない」絵
		constexpr std::string_view EMPTY = "ext-emp";
	} // namespace extension_icon_id

	/**
	 * @brief 拡張子種別に対応するアイコンの画像IDを返す
	 *
	 * 装備スロット・落ちている欠片・背景のパーティクルが同じ絵を使う。
	 * 対応表を1つに保つことで、「壊したブロックの絵」と「スロットに入る絵」が
	 * 食い違わないようにする
	 * @param type 拡張子種別
	 * @return 画像ID（resources.json の images[].id）
	 */
	[[nodiscard]] constexpr std::string_view toExtensionIconId(core::data::FileExtensionType type) noexcept
	{
		switch (type)
		{
		case core::data::FileExtensionType::Executable: return "ext-exe";
		case core::data::FileExtensionType::Document: return "ext-doc";
		case core::data::FileExtensionType::Image: return "ext-img";
		case core::data::FileExtensionType::Audio: return "ext-aud";
		case core::data::FileExtensionType::SourceCode: return "ext-src";
		case core::data::FileExtensionType::Shortcut: return "ext-lnk";
		case core::data::FileExtensionType::Video: return "ext-vid";
		case core::data::FileExtensionType::Archive: return "ext-arc";
		default: return "ext-etc";
		}
	}
} // namespace game::constant
