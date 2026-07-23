#pragma once
#include <string>
#include <string_view>
#include <algorithm>
#include <array>
#include "game/data/FileExtensionType.h"
#include "game/utility/FileExtensionTypeResolver.h"

namespace game::data
{
	/**
	 * @brief セレクト画面で選択したファイルの情報を保持するクラス
	 * GameManager が唯一の所有者となり、シーン間のデータ受け渡しに使用する
	 */
	class FileEquipmentData
	{
	public:
		static constexpr int MAX_SLOTS{3};

		/**
		 * @brief 選択ファイルのパスをセットする
		 * 拡張子を解析して FileExtensionType を自動設定する
		 * @param slotIndex スロット番号（0〜MAX_SLOTS-1）  ← 追加
		 * @param path ファイルのフルパス
		 */
		void setFilePath(int slotIndex, std::string_view path)
		{
			if (slotIndex < 0 || slotIndex >= MAX_SLOTS)
				return;
			m_filePaths[slotIndex] = path;
			m_hasSelection[slotIndex] = true;

			m_extensionTypes[slotIndex] = utility::FileExtensionTypeResolver::fromPath(path);
		}

		/**
		 * @brief 指定スロットの選択をクリアする
		 * @param slotIndex スロット番号（0〜MAX_SLOTS-1）
		 */
		void clearSlot(int slotIndex)
		{
			if (slotIndex < 0 || slotIndex >= MAX_SLOTS)
				return;
			m_filePaths[slotIndex] = {};
			m_extensionTypes[slotIndex] = FileExtensionType::Unknown;
			m_hasSelection[slotIndex] = false;
		}

		/** @brief 全スロットをクリアする */
		void clearAll()
		{
			for (int i{0}; i < MAX_SLOTS; ++i)
				clearSlot(i);
		}

		/** @brief 指定スロットが選択済みか */
		[[nodiscard]] bool hasSelection(int slotIndex) const noexcept
		{
			if (slotIndex < 0 || slotIndex >= MAX_SLOTS)
				return false;
			return m_hasSelection[slotIndex];
		}

		/** @brief 指定スロットのフルパスを取得 */
		[[nodiscard]] const std::string &getFilePath(int slotIndex) const noexcept
		{
			return m_filePaths[slotIndex];
		}

		/** @brief 指定スロットの拡張子種別を取得 */
		[[nodiscard]] FileExtensionType getExtensionType(int slotIndex) const noexcept
		{
			return m_extensionTypes[slotIndex];
		}

	private:
		std::array<std::string, MAX_SLOTS> m_filePaths{};
		std::array<FileExtensionType, MAX_SLOTS> m_extensionTypes{FileExtensionType::Unknown, FileExtensionType::Unknown, FileExtensionType::Unknown};
		std::array<bool, MAX_SLOTS> m_hasSelection{false, false, false};
	};
} // namespace game::data