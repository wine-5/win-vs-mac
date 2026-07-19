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
		R, // DEBUG: デバッグ用
		T, // DEBUG: テストエフェクト再生
		Space,
        Escape, // TODO:将来的にPauseにする予定

        Up,
        Down,
        Left,
        Right,

		Shift, // ダッシュ用
		Tab,   // ロックオン切り替え用
	};
} // namespace core::input