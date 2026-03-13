#pragma once

namespace game::scene
{
    /**
     * @brief シーンの種類を定義
     */
    enum class SceneType
    {
        Title,
        StageSelect,
        Loading,
        InGame,
        Result
    };
}