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

		// 殴って壊せる配置物。床・壁と見た目は同じ立方体だが、攻撃の対象になる点が違う。
		Destructible,
	};
} // namespace game::constant