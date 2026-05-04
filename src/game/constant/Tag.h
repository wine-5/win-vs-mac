#pragma once

namespace game::constant
{
    /**
     * @brief 衝突判定のタグ
     * オブジェクトの種類を識別する
     */
    enum class Tag
    {
        None,
        Player,
        Enemy,
        Ground,
        Wall,
    };
}