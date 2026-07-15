#pragma once

namespace game::data
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
        Archive,    // .zip .7z .rar   → 全パラメータ小+
        Unknown     // それ以外        → attackRange+
    };
} // namespace game::data