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
		T, // DEBUG: テストエフェクト再生
		Space,
		Enter,  // 決定（ポーズメニュー等）
		Escape, // ポーズメニューの開閉（Biosではスキップ）

		Up,
        Down,
        Left,
        Right,

		Shift, // ダッシュ用
	};
} // namespace core::input