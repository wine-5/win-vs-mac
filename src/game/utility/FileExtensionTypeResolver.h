#pragma once
#include <algorithm>
#include <string>
#include <string_view>
#include "core/data/FileExtensionType.h"

namespace game::utility
{
    /**
     * @brief ファイル拡張子文字列を FileExtensionType に変換するクラス
     */
	class FileExtensionTypeResolver
	{
    public:
	  /** @brief 拡張子1つと、それが属する種別の対応 */
	  struct Entry
	  {
		  std::string_view m_extension;
		  core::data::FileExtensionType m_type;
	  };

	  /**
	   * @brief 拡張子と種別の対応表
	   *
	   * 判定にも一覧表示にも同じ表を使う。ここだけを直せば、
	   * 装備時の効果とセレクト画面の説明が食い違わない
	   */
	  static constexpr Entry EXTENSION_TABLE[]{
		  { ".exe", core::data::FileExtensionType::Executable },
		  { ".dll", core::data::FileExtensionType::Executable },
		  { ".bat", core::data::FileExtensionType::Executable },
		  { ".txt", core::data::FileExtensionType::Document },
		  { ".pdf", core::data::FileExtensionType::Document },
		  { ".docx", core::data::FileExtensionType::Document },
		  { ".png", core::data::FileExtensionType::Image },
		  { ".jpg", core::data::FileExtensionType::Image },
		  { ".bmp", core::data::FileExtensionType::Image },
		  { ".mp3", core::data::FileExtensionType::Audio },
		  { ".wav", core::data::FileExtensionType::Audio },
		  { ".flac", core::data::FileExtensionType::Audio },
		  { ".cpp", core::data::FileExtensionType::SourceCode },
		  { ".h", core::data::FileExtensionType::SourceCode },
		  { ".py", core::data::FileExtensionType::SourceCode },
		  { ".js", core::data::FileExtensionType::SourceCode },
		  { ".cs", core::data::FileExtensionType::SourceCode },
		  { ".lnk", core::data::FileExtensionType::Shortcut },
		  { ".url", core::data::FileExtensionType::Shortcut },
		  { ".mp4", core::data::FileExtensionType::Video },
		  { ".avi", core::data::FileExtensionType::Video },
		  { ".mkv", core::data::FileExtensionType::Video },
		  { ".mov", core::data::FileExtensionType::Video },
		  { ".zip", core::data::FileExtensionType::Archive },
		  { ".7z", core::data::FileExtensionType::Archive },
		  { ".rar", core::data::FileExtensionType::Archive },
	  };

	  /**
	   * @brief 拡張子文字列を FileExtensionType に変換する
	   * @param ext 拡張子（例：".exe"）※小文字前提
	   * @return 対応する FileExtensionType（表に無ければ Unknown）
	   */
	  [[nodiscard]] static constexpr core::data::FileExtensionType toFileExtensionType(std::string_view ext) noexcept
	  {
		  for (const auto& entry : EXTENSION_TABLE)
		  {
			  if (entry.m_extension == ext)
				  return entry.m_type;
		  }
		  return core::data::FileExtensionType::Unknown;
		}

		/**
		 * @brief 種別に属する拡張子を並べた文字列を返す
		 *
		 * セレクト画面の「拡張子ボーナス一覧」で、どのファイルが対象なのかを
		 * 省略せずに見せるために使う
		 * @param type 拡張子種別
		 * @param separator 拡張子どうしの区切り
		 * @return 例 ".exe, .dll, .bat"（該当が無ければ空文字）
		 */
		[[nodiscard]] static std::string joinExtensions(core::data::FileExtensionType type,
		    std::string_view separator = ", ")
		{
			std::string result{};
			for (const auto& entry : EXTENSION_TABLE)
			{
				if (entry.m_type != type)
					continue;
				if (!result.empty())
					result += separator;
				result += entry.m_extension;
			}
			return result;
		}

		/**
		 * @brief ファイルパスから FileExtensionType を判定する
		 *
		 * 「末尾の . 以降を切り出して小文字化してから解決する」という手順を
		 * 呼び出し側で書き写さなくて済むよう、ここを唯一の実装とする。
		 * @param path ファイルパス（拡張子の大文字小文字は問わない）
		 * @return 対応する FileExtensionType（拡張子が無い場合は Unknown）
		 */
		[[nodiscard]] static core::data::FileExtensionType fromPath(std::string_view path)
		{
			const auto dotPos{ path.rfind('.') };
			if (dotPos == std::string_view::npos)
				return core::data::FileExtensionType::Unknown;

			std::string ext{ path.substr(dotPos) };
			std::transform(ext.begin(), ext.end(), ext.begin(),
			    [](unsigned char c)
			    { return static_cast<char>(std::tolower(c)); });
			return toFileExtensionType(ext);
		}
	};

} // namespace game::utility