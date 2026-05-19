#pragma once

namespace game::scene
{
    /**
     * @brief シーンの種類を定義
     */
    enum class SceneType
    {
        Bios,       // タイトル前のBIOS画面
        Lockscreen, // ロック画面
        Title,      // タイトル画面
        Select,     // セレクト画面
        Loading,    // ローディング画面
        InGame,     // ゲーム画面
        Result      // リザルト画面
    };
}