#pragma once

namespace game::constant
{
    /**
     * @brief 衝突判定のタグ
     * オブジェクトの種類を識別する
     */
    enum class CollisionTag
    {
        None,
        Player,
        Enemy,
        Ground,
        Wall,
    };
}