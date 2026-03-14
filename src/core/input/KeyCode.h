#pragma once

namespace core::input
{
    /**
     * @brief DxLibに依存しない独自のキーコードを定義
     */

    enum class KeyCode
    {
        W,
        A,
        S,
        D,
        R, // デバック用
        Space,
        Escape, // TODO:将来的にPauseにする予定

        Up,
        Down,
        Left,
        Right,
    };
}