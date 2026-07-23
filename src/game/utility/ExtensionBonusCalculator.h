#pragma once
#include "core/data/FileExtensionType.h"
#include "core/data/FileExtensionBonus.h"

namespace game::utility
{
	namespace
	{
		constexpr float EXECUTABLE_ATK{ 10.0f };
		constexpr float DOCUMENT_SPD{ 1.5f };
		constexpr float IMAGE_DEF{ 8.0f };
		constexpr float AUDIO_HP{ 20.0f };
		constexpr float ARCHIVE_ATK{ 3.0f };
		constexpr float ARCHIVE_SPD{ 0.5f };
		constexpr float ARCHIVE_DEF{ 3.0f };
		constexpr float ARCHIVE_HP{ 5.0f };
		constexpr float ARCHIVE_RANGE{ 0.5f };
		constexpr float UNKNOWN_RANGE{ 20.0f };
	} // namespace

	/**
	 * @brief FileExtensionType に応じたパラメータボーナスを算出するクラス
	 */
	class ExtensionBonusCalculator
	{
	public:
		/**
		 * @brief 拡張子種別に対応する FileExtensionBonus を返す
		 * @param type ファイル拡張子グループ種別
		 * @return 対応するボーナス値
		 */
	  [[nodiscard]] static constexpr core::data::FileExtensionBonus calculate(core::data::FileExtensionType type) noexcept
	  {
		  switch (type)
		  {
		  case core::data::FileExtensionType::Executable:
			  return { .atk = EXECUTABLE_ATK };
		  case core::data::FileExtensionType::Document:
			  return { .spd = DOCUMENT_SPD };
		  case core::data::FileExtensionType::Image:
			  return { .def = IMAGE_DEF };
		  case core::data::FileExtensionType::Audio:
			  return { .hp = AUDIO_HP };
		  case core::data::FileExtensionType::Archive:
			  return { .atk = ARCHIVE_ATK, .spd = ARCHIVE_SPD, .def = ARCHIVE_DEF, .hp = ARCHIVE_HP, .attackRange = ARCHIVE_RANGE };
		  case core::data::FileExtensionType::Unknown:
		  default:
			  return { .attackRange = UNKNOWN_RANGE };
		  }
		}
	};
} // namespace game::utility