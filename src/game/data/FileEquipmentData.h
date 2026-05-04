#pragma once
#include <string>
#include <string_view>
#include <algorithm>
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
		/**
		 * @brief 選択ファイルのパスをセットする
		 * 拡張子を解析して FileExtensionType を自動設定する
		 * @param path ファイルのフルパス
		 */
		void setFilePath(std::string_view path)
		{
			m_selectedFilePath = path;
			m_hasSelection = true;

			const auto dotPos = path.rfind('.'); // 後ろから.を探す
			if (dotPos != std::string_view::npos) // 見つかった場合
			{
				std::string ext{ path.substr(dotPos) };
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
				m_extensionType = utility::FileExtensionTypeResolver::toFileExtensionType(ext);
			}
			else
				m_extensionType = FileExtensionType::Unknown;
		}

		/**
		 * @brief 選択をクリアする
		 */
		void clearSelection()
		{
			m_selectedFilePath = {};
			m_hasSelection = false;
			m_extensionType = FileExtensionType::Unknown;
		}

		/** @brief 選択されたファイルのフルパスを取得 */
		[[nodiscard]] const std::string& getFilePath() const noexcept { return m_selectedFilePath; }

		/** @brief 拡張子グループ種別を取得 */
		[[nodiscard]] FileExtensionType getExtensionType() const noexcept { return m_extensionType; }

		/** @brief ファイルが選択されているか */
		[[nodiscard]] bool hasSelection() const noexcept { return m_hasSelection; }

	private:
		std::string m_selectedFilePath{};
		FileExtensionType m_extensionType{ FileExtensionType::Unknown };
		bool m_hasSelection{ false };
	};
}