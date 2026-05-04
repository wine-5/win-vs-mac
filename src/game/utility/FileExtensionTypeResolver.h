#pragma once
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
	};

}