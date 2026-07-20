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
		R,  // DEBUG: デバッグ用（現在未使用）
		T,  // DEBUG: テストエフェクト再生
		F1, // DEBUG: デバッグモードのON/OFF切り替え
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