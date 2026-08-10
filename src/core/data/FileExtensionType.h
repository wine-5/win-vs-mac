#pragma once
#include <string_view>
#include <utility>

namespace core::data
{
    /**
    * @brief ファイル拡張子のグループ種別
    */
    enum class FileExtensionType
    {
        Executable, // .exe .dll .bat  → ATK+
        Document,   // .txt .pdf .docx → SPD+
        Image,      // .png .jpg .bmp  → DEF+
        Audio,      // .mp3 .wav .flac → HP+
		SourceCode, // .cpp .h .py .js → クリティカル率+
		Shortcut,   // .lnk .url       → Window弾の弾速+
		Video,      // .mp4 .avi .mkv  → Window弾の飛距離+
		Archive,    // .zip .7z .rar   → 全パラメータ小+
		Unknown,    // それ以外        → attackRange+

		// 種別の総数。種別ごとの配列を確保する側が使う。
		// 必ず末尾に置くこと（種別を追加しても配列サイズが自動で追従する）
		Count
	};

	/**
	 * @brief JSONに書く種別名と列挙の対応表
	 *
	 * extensionBonus.json のキーと、stageCatalog.json のドロップ指定が同じ名前を使う。
	 * 表を1つに保つことで、種別を足したときに片方だけ古いまま残ることを防ぐ
	 */
	inline constexpr std::pair<std::string_view, FileExtensionType> EXTENSION_TYPE_NAMES[]{
		{ "executable", FileExtensionType::Executable },
		{ "document", FileExtensionType::Document },
		{ "image", FileExtensionType::Image },
		{ "audio", FileExtensionType::Audio },
		{ "sourceCode", FileExtensionType::SourceCode },
		{ "shortcut", FileExtensionType::Shortcut },
		{ "video", FileExtensionType::Video },
		{ "archive", FileExtensionType::Archive },
		{ "unknown", FileExtensionType::Unknown },
	};

	/**
	 * @brief 種別名を FileExtensionType へ変換する
	 * @param name 種別名（例: "image"）
	 * @return 対応する種別。表に無ければ Unknown
	 */
	[[nodiscard]] constexpr FileExtensionType toExtensionType(std::string_view name) noexcept
	{
		for (const auto& [key, type] : EXTENSION_TYPE_NAMES)
		{
			if (key == name)
				return type;
		}
		return FileExtensionType::Unknown;
	}
} // namespace core::data