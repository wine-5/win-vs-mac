#pragma once
#include <algorithm>
#include <string>
#include <string_view>
#include "game/data/FileExtensionType.h"

namespace game::utility
{
    /**
     * @brief ファイル拡張子文字列を FileExtensionType に変換するクラス
     */
	class FileExtensionTypeResolver
	{
    public:
		/**
		  * @brief 拡張子文字列を FileExtensionType に変換する
		  * @param ext 拡張子（例：".exe"）※小文字前提
		  * @return 対応する FileExtensionType
		  */
		[[nodiscard]] static constexpr data::FileExtensionType toFileExtensionType(std::string_view ext) noexcept
        {
            if (ext == ".exe" || ext == ".dll" || ext == ".bat")
                return data::FileExtensionType::Executable;
            if (ext == ".txt" || ext == ".pdf" || ext == ".docx")
                return data::FileExtensionType::Document;
            if (ext == ".png" || ext == ".jpg" || ext == ".bmp")
                return data::FileExtensionType::Image;
            if (ext == ".mp3" || ext == ".wav" || ext == ".flac")
                return data::FileExtensionType::Audio;
            if (ext == ".zip" || ext == ".7z" || ext == ".rar")
                return data::FileExtensionType::Archive;
            return data::FileExtensionType::Unknown;
        }

		/**
		 * @brief ファイルパスから FileExtensionType を判定する
		 *
		 * 「末尾の . 以降を切り出して小文字化してから解決する」という手順を
		 * 呼び出し側で書き写さなくて済むよう、ここを唯一の実装とする。
		 * @param path ファイルパス（拡張子の大文字小文字は問わない）
		 * @return 対応する FileExtensionType（拡張子が無い場合は Unknown）
		 */
		[[nodiscard]] static data::FileExtensionType fromPath(std::string_view path)
		{
			const auto dotPos{ path.rfind('.') };
			if (dotPos == std::string_view::npos)
				return data::FileExtensionType::Unknown;

			std::string ext{ path.substr(dotPos) };
			std::transform(ext.begin(), ext.end(), ext.begin(),
			    [](unsigned char c)
			    { return static_cast<char>(std::tolower(c)); });
			return toFileExtensionType(ext);
		}
	};

} // namespace game::utility