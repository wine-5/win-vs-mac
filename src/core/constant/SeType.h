#pragma once

namespace core::constant
{
    /**
     * @brief SE（効果音）の種類を表す列挙型
     */
    enum class SeType
    {
        None,

        // ゲーム関連
		AttackPlayer,    // プレイヤーの攻撃
		HitEnemy,        // 敵がダメージを受けた
		HitPlayer,       // プレイヤーがダメージを受けた
        DeadEnemy,       // 敵が倒れた

        // 今後追加したら AudioRepository の typeMap にも追加を忘れないように
    };
} // namespace core::constant