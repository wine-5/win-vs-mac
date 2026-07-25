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
		T,  // DEBUG: テストエフェクト再生
		F1, // DEBUG: デバッグモードのON/OFF切り替え
		F2, // DEBUG: シーンビュー（時間停止＋フリーカメラ）のON/OFF切り替え
		F3, // DEBUG: 連続ジャンプ（空中浮上）のON/OFF切り替え
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