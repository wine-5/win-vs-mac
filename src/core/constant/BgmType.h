#pragma once

namespace core::constant
{
    /**
     * @brief BGM の種類を表す列挙型
     */
    enum class BgmType
    {
        None,
        Title,      // タイトル画面
        Select,     // ジョブ選択画面
        InGame,     // ゲーム中（通常）
        Boss,       // ボス戦
        ResultWin,  // リザルト（勝利）
        ResultLose, // リザルト（敗北）
    };
}