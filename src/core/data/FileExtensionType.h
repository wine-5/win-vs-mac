#pragma once

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
		SourceCode, // .cpp .h .py .js → 会心率+
		Shortcut,   // .lnk .url       → Window弾の弾速+
		Video,      // .mp4 .avi .mkv  → Window弾の飛距離+
		Archive,    // .zip .7z .rar   → 全パラメータ小+
		Unknown,    // それ以外        → attackRange+

		// 種別の総数。種別ごとの配列を確保する側が使う。
		// 必ず末尾に置くこと（種別を追加しても配列サイズが自動で追従する）
		Count
	};
} // namespace core::data