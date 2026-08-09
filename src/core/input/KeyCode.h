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
		Space,
		Enter,  // 決定（ポーズメニュー等）
		Escape, // ポーズメニューの開閉（Biosではスキップ）

		Up,
        Down,
        Left,
        Right,

		Shift, // ダッシュ用
		Tab,   // 押している間だけステータス一覧を開く
		E,     // インベントリの開閉
	};
} // namespace core::input