#pragma once

namespace core::input
{
    /**
     * @brief コントローラーの入力コードの定義
     */
    enum class GamePadCode
    {
        // スティック
        LeftStickX,
        LeftStickY,

        // ボタン
        ButtonA, // 攻撃（〇）
        ButtonB, // ジャンプ（×）

        // 十字
        DPadUp,
        DPadDown,
        DPadLeft,
        DPadRight,
    };
}