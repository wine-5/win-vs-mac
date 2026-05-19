#pragma once

namespace game::scene
{
    /**
     * @brief シーンの種類を定義
     */
    enum class SceneType
    {
        Lockscreen,
        Title,
        Select,
        Loading,
        InGame,
        Result
    };
}