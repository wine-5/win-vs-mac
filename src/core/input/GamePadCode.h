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
        ButtonB, // ジャンプ

        // 十字
        DPadUp,
        DPadDown,
        DPadLeft,
        DPadRight,
    };
}