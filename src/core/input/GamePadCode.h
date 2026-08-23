#pragma once

namespace core::input
{
	/**
	 * @brief ゲームパッドの入力コードの定義
	 *
	 * ボタン名は PlayStation 表記で持つ。Xbox 配列のパッドでも
	 * 同じ位置のボタンへ割り当てる（×=A・〇=B・□=X・△=Y）。
	 *
	 * スティックとトリガーは getPadAxis、それ以外は isPadButtonDown で読む。
	 * トリガーは押し切りをボタンとしても拾えるよう、両方の口を用意している
	 */
	enum class GamePadCode
	{
		// ========== アナログ（getPadAxis で読む） ==========

		LeftStickX,  // 左スティック横（右が正）
		LeftStickY,  // 左スティック縦（前＝奥が正。DxLibの符号はInputManagerで吸収する）
		RightStickX, // 右スティック横（右が正）
		RightStickY, // 右スティック縦（上が正）
		AxisL2,      // L2の踏み込み（0.0〜1.0）
		AxisR2,      // R2の踏み込み（0.0〜1.0）

		// ========== ボタン（isPadButtonDown で読む） ==========

		// 面ボタン
		ButtonCross,    // ×（Xbox: A）
		ButtonCircle,   // 〇（Xbox: B）
		ButtonSquare,   // □（Xbox: X）
		ButtonTriangle, // △（Xbox: Y）

		// 肩・トリガー
		ButtonL1, // L1（Xbox: LB）
		ButtonR1, // R1（Xbox: RB）
		ButtonL2, // L2を押し切ったか（Xbox: LT）
		ButtonR2, // R2を押し切ったか（Xbox: RT）

		// スティック押し込み
		ButtonL3, // 左スティック押し込み（Xbox: LS）
		ButtonR3, // 右スティック押し込み（Xbox: RS）

		// システム
		ButtonOptions, // OPTIONS（Xbox: Menu／Start）
		ButtonShare,   // SHARE（Xbox: View／Back）

		// 十字キー
		DPadUp,
		DPadDown,
		DPadLeft,
		DPadRight,

		Count,
	};
} // namespace core::input